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
    msgHandlerMap_.emplace(P2P_MSG, bind(&ChatService::p2pChat, this, _1, _2, _3));
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
    response["msgid"] = REG_MSG_ACK;
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
            response["offlinemessages"]=offlineMsgModel_.query(id);

            //清除离线消息
            offlineMsgModel_.erase(id);
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
    }
    else
    {
        // 注册失败
        response["errno"] = 1;
    }
    conn->send(response.dump());
}

// 点对点聊天业务
void ChatService::p2pChat(const muduo::net::TcpConnectionPtr &conn,
                          nlohmann::json &js,
                          muduo::Timestamp time)
{
    int toid = js["to"].get<int>();
    bool isoff = false;

    {
        unique_lock<mutex> lock(mtx_);
        auto it = userConnMap_.find(toid);
        if (it != userConnMap_.end())
        {
            // toid 在线，由服务端转发消息
            it->second->send(js.dump());
            return;
        }
        isoff = true;
    }

    if (isoff)
    {
        // toid 不在线，存储离线消息
        offlineMsgModel_.insert(toid,js.dump());
    }
}