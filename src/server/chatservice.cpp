#include <iostream>

#include "chatservice.hpp"
#include "common.hpp"

using namespace std;
using namespace std::placeholders;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// 构造函数
ChatService::ChatService()
{
    // 增加回调函数
    msgHandlerMap_.emplace(LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3));
    msgHandlerMap_.emplace(REG_MSG, bind(&ChatService::reg, this, _1, _2, _3));
    msgHandlerMap_.emplace(P2P_CHAT_MSG, bind(&ChatService::p2pChat, this, _1, _2, _3));
    msgHandlerMap_.emplace(ADD_FRIEND_REQ_MSG, bind(&ChatService::addFriendReq, this, _1, _2, _3));
    msgHandlerMap_.emplace(ADD_FRIEND_VERIFY_MSG, bind(&ChatService::addFriendVerify, this, _1, _2, _3));
    msgHandlerMap_.emplace(CREATE_GROUP_MSG, bind(&ChatService::createGroup, this, _1, _2, _3));
    msgHandlerMap_.emplace(ADD_GROUP_MSG, bind(&ChatService::addGroup, this, _1, _2, _3));
    msgHandlerMap_.emplace(GROUP_CHAR_MSG, bind(&ChatService::groupChat, this, _1, _2, _3));
}

// 获取实例
ChatService *ChatService::getInstance()
{
    static ChatService csv;
    return &csv;
}

// 获取回调函数
MsgHandler ChatService::getHandler(int msgid)
{
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end())
    {
        return [=](const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp time) -> void
        {
            LOG_INFO << "not find msgHandler for type: " << msgid;
        };
    }
    return it->second;
}

// 客户端下线
void ChatService::clientLogout(const muduo::net::TcpConnectionPtr &conn)
{
    User user;

    {
        unique_lock<mutex> lock(mtx_);
        for (auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }

    if (user.getId() != -1)
    {
        user.setState("offline");
        userModel_.updateState(user);
    }
}

// 登录业务 id password
void ChatService::login(const muduo::net::TcpConnectionPtr &conn,
                        nlohmann::json &js,
                        muduo::Timestamp time)
{
    // cout << "这是登录函数" << endl;
    int id = js["id"].get<int>();
    string password = js["password"].get<string>();

    User user = userModel_.query(id);
    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    if (user.getId() != -1 && user.getPassword() == password)
    {
        // 账号，密码验证成功
        if (user.getState() == "online")
        {
            // 已经在线
            response["errno"] = 2;
            response["errmsg"] = "用户已经在线,重复登录";
        }
        else
        {
            // 登录成功
            // 增加一个长连接
            {
                unique_lock<mutex> lock(mtx_);
                userConnMap_.emplace(id, conn);
            }

            // 更新在线状态
            user.setState("online");
            userModel_.updateState(user);

            // 回发客户端
            response["errno"] = 0;
            response["id"] = user.getId();
            response["username"] = user.getUsername();

            // 离线消息
            vector<string> vec_offlinemsg = offlineMsgModel_.query(id);
            if (!vec_offlinemsg.empty())
            {
                response["offlinemessages"] = vec_offlinemsg;
                // 清除离线消息
                offlineMsgModel_.erase(id);
            }

            // 未确认的好友请求
            vector<int> vec_fri_unv = friendModel_.queryUnverify(id);
            if (!vec_fri_unv.empty())
            {
                response["friendunverifys"] = vec_fri_unv;
            }

            // 已经添加的群组
            vector<Group> vec_gro = groupModel_.queryGroups(id);
            if (!vec_gro.empty())
            {
                vector<string> vec;
                for (auto &group : vec_gro)
                {
                    json js;
                    js["groupid"] = group.getId();
                    js["groupname"] = group.getGroupname();
                    js["groupdesc"] = group.getGroupdesc();
                    vec.emplace_back(js.dump());
                }
                response["groups"] = vec;
            }

            // 好友信息
            vector<User> vec_fri = friendModel_.queryFriends(id);
            if (!vec_fri.empty())
            {
                vector<string> vec;
                for (auto &user : vec_fri)
                {
                    json js;
                    js["id"] = user.getId();
                    js["username"] = user.getUsername();
                    js["state"] = user.getState();
                    vec.emplace_back(js.dump());
                }
                response["friends"] = vec;
            }
            response["sucmsg"] = "登录成功";
        }
    }
    else
    {
        // 账号，密码验证失败
        response["errno"] = 1;
        response["errmsg"] = "用户账号或者密码错误";
    }
    conn->send(response.dump());
}

// 注册业务 username password
void ChatService::reg(const muduo::net::TcpConnectionPtr &conn,
                      nlohmann::json &js,
                      muduo::Timestamp time)
{
    // cout << "这是注册函数" << endl;
    string username = js["username"].get<string>();
    string password = js["password"].get<string>();

    // 声明对象
    User user;
    user.setUsername(username);
    user.setPassword(password);

    // 执行操作
    int state = userModel_.insert(user);
    json response;
    response["msgid"] = REG_MSG_ACK;
    if (state)
    {
        // 注册成功
        response["errno"] = 0;
        response["id"] = user.getId();
        response["username"] = user.getUsername();
        response["sucmsg"] = "注册成功";
    }
    else
    {
        // 注册失败
        response["errno"] = 1;
        response["errmsg"] = "注册失败";
    }
    conn->send(response.dump());
}

// 点对点聊天业务
void ChatService::p2pChat(const muduo::net::TcpConnectionPtr &conn,
                          nlohmann::json &js,
                          muduo::Timestamp time)
{
    int toid = js["to"].get<int>();

    json response;
    response["msgid"] = P2P_CHAT_MSG_ACK;
    // 用户不存在
    User user = userModel_.query(toid);
    if (user.getId() == -1)
    {
        response["errno"] = 1;
        response["errmsg"] = "用户账号不存在";
        conn->send(response.dump());
        return;
    }

    // 用户存在
    // 是否离线
    bool isoff = false;

    {
        unique_lock<mutex> lock(mtx_);
        auto it = userConnMap_.find(toid);
        if (it != userConnMap_.end())
        {
            // toid 在线，由服务端转发消息
            it->second->send(js.dump());
        }
        else
        {
            isoff = true;
        }
    }

    response["errno"] = 0;

    if (isoff)
    {
        // toid 不在线，存储离线消息
        offlineMsgModel_.insert(toid, js.dump());
        response["sucmsg"] = "发送离线消息成功";
    }
    else
    {
        response["sucmsg"] = "发送消息成功";
    }

    conn->send(response.dump());
}

// 添加好友请求业务
void ChatService::addFriendReq(const muduo::net::TcpConnectionPtr &conn,
                               nlohmann::json &js,
                               muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    string username = js["from"].get<string>();
    int friendid = js["to"].get<int>();

    json response;
    response["msgid"] = ADD_FRIEND_REQ_MSG_ACK;
    // 用户不存在
    User user = userModel_.query(friendid);
    if (user.getId() == -1)
    {
        response["errno"] = 1;
        response["errmsg"] = "用户账号不存在";
        conn->send(response.dump());
        return;
    }

    // 用户存在
    // 新增一条(未确认)
    friendModel_.insert(userid, friendid);
    // 增加描述
    js["desc"] = username + "请求跟你添加好友";

    // 发送验证消息
    // 是否离线
    bool isoff = false;

    {
        unique_lock<mutex> lock(mtx_);
        auto it = userConnMap_.find(friendid);
        if (it != userConnMap_.end())
        {
            // friendid 在线
            it->second->send(js.dump());
        }
        else
        {
            isoff = true;
        }
    }

    if (isoff)
    {
        // friendid 不在线
        offlineMsgModel_.insert(friendid, js.dump());
    }

    response["errno"] = 0;
    response["sucmsg"] = "发送验证消息成功";
    conn->send(response.dump());
}

// 添加好友验证业务
void ChatService::addFriendVerify(const muduo::net::TcpConnectionPtr &conn,
                                  nlohmann::json &js,
                                  muduo::Timestamp time)
{
    int userid = js["to"].get<int>();
    int friendid = js["id"].get<int>();
    string friendname = js["from"].get<string>();
    bool agree = js["agree"].get<bool>();

    json res_user, res_fri;
    res_user["msgid"] = ADD_FRIEND_VERIFY_MSG_ACK;
    res_fri["msgid"] = ADD_FRIEND_VERIFY_MSG_ACK;
    if (agree)
    {
        // 同意
        res_user["desc"] = friendname + "通过你的好友请求";
        // 更新数据库
        friendModel_.pass(userid, friendid);
    }
    else
    {
        // 不同意
        res_user["desc"] = friendname + "没有通过你的好友请求";
        // 更新数据库
        friendModel_.unpass(userid, friendid);
    }
    res_fri["desc"] = "成功处理好友请求";
    // 回发客户端
    conn->send(res_fri.dump());

    // 发送给好友请求的客户
    // 是否离线
    bool isoff = false;

    {
        unique_lock<mutex> lock(mtx_);
        auto it = userConnMap_.find(userid);
        if (it != userConnMap_.end())
        {
            // userid 在线
            it->second->send(res_user.dump());
        }
        else
        {
            isoff = true;
        }
    }

    if (isoff)
    {
        // userid 不在线
        offlineMsgModel_.insert(userid, res_user.dump());
    }
}

// 创建群组业务
void ChatService::createGroup(const muduo::net::TcpConnectionPtr &conn,
                              nlohmann::json &js,
                              muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    string groupname = js["groupname"].get<string>();
    string groupdesc = js["groupdesc"].get<string>();

    Group group(-1, groupname, groupdesc);

    json response;
    response["msgid"] = CREATE_GROUP_MSG_ACK;
    if (groupModel_.createGroup(group))
    {
        // 创建群组成功
        int groupid = group.getId();
        // 更新数据库
        groupModel_.addGroup(groupid, userid, "owner");

        response["groupid"] = groupid;
        response["errno"] = 0;
        response["sucmsg"] = "创建群组成功";
    }
    else
    {
        // 创建群组失败
        response["errno"] = 1;
        response["errmsg"] = "创建群组失败";
    }
    conn->send(response.dump());
}

// 添加群组业务
void ChatService::addGroup(const muduo::net::TcpConnectionPtr &conn,
                           nlohmann::json &js,
                           muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    json response;
    response["msgid"] = ADD_GROUP_MSG_ACK;

    Group group = groupModel_.queryGroup(groupid);
    if (group.getId() == -1)
    {
        response["errno"] = 1;
        response["errmsg"] = "群组不存在";
        conn->send(response.dump());
        return;
    }

    // 更新数据库
    if (!groupModel_.addGroup(groupid, userid))
    {
        response["errno"] = 2;
        response["errmsg"] = "添加群组失败";
        conn->send(response.dump());
        return;
    }

    response["errno"] = 0;
    response["sucmsg"] = "添加群组" + group.getGroupname() + "成功";
    conn->send(response.dump());
}

// 群组聊天业务
void ChatService::groupChat(const muduo::net::TcpConnectionPtr &conn,
                            nlohmann::json &js,
                            muduo::Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    vector<GroupUser> groupusers = groupModel_.queryGroupUsers(groupid, userid);

    if (!groupusers.empty())
    {
        // 发送群聊消息
        for (auto &groupuser : groupusers)
        {
            int toid = groupuser.getId();
            // 是否离线
            bool isoff = false;
            {
                unique_lock<mutex> lock(mtx_);
                auto it = userConnMap_.find(toid);
                if (it != userConnMap_.end())
                {
                    it->second->send(js.dump());
                }
                else
                {
                    isoff = true;
                }
            }
            if (isoff)
            {
                offlineMsgModel_.insert(toid, js.dump());
            }
        }
    }
}