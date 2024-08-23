#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <functional>
#include <unordered_map>
#include <mutex>

#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp time)>;

/**
 * @brief 单例业务类
 * 提供业务函数
 * 注册回调函数
 * 获取回调函数
 */
class ChatService
{
public:
    // 获取实例
    static ChatService *getInstance();

    // 获取回调函数
    MsgHandler getHandler(int msgid);

    // 客户端下线
    void clientLogout(const muduo::net::TcpConnectionPtr &conn);

protected:
    // 登录业务
    void login(const muduo::net::TcpConnectionPtr &conn,
               nlohmann::json &js,
               muduo::Timestamp time);

    // 注册业务
    void reg(const muduo::net::TcpConnectionPtr &conn,
             nlohmann::json &js,
             muduo::Timestamp time);

    // 点对点聊天业务
    void p2pChat(const muduo::net::TcpConnectionPtr &conn,
                 nlohmann::json &js,
                 muduo::Timestamp time);

    // 添加好友请求业务
    void addFriendReq(const muduo::net::TcpConnectionPtr &conn,
                      nlohmann::json &js,
                      muduo::Timestamp time);

    // 添加好友验证业务
    void addFriendVerify(const muduo::net::TcpConnectionPtr &conn,
                         nlohmann::json &js,
                         muduo::Timestamp time);

    // 创建群组业务
    void createGroup(const muduo::net::TcpConnectionPtr &conn,
                         nlohmann::json &js,
                         muduo::Timestamp time);
    
    // 添加群组业务
    void addGroup(const muduo::net::TcpConnectionPtr &conn,
                         nlohmann::json &js,
                         muduo::Timestamp time);

    // 群组聊天业务
    void groupChat(const muduo::net::TcpConnectionPtr &conn,
                         nlohmann::json &js,
                         muduo::Timestamp time);

    // 刷新业务
    void refresh(const muduo::net::TcpConnectionPtr &conn,
                         nlohmann::json &js,
                         muduo::Timestamp time);

private:
    // 构造函数
    ChatService();

    ChatService(const ChatService &) = default;
    ChatService &operator=(const ChatService &) = default;
    ChatService(ChatService &&) = default;
    ChatService &operator=(ChatService &&) = default;

    std::unordered_map<int, MsgHandler> msgHandlerMap_;                 ///< 存储回调函数
    std::unordered_map<int, muduo::net::TcpConnectionPtr> userConnMap_; ///< 维护长连接
    std::mutex mtx_;                                                    ///< 维护长连接表的线程安全锁

    // 数据库操作对象
    UserModel userModel_;
    OfflineMsgModel offlineMsgModel_;
    FriendModel friendModel_;
    GroupModel groupModel_;
    Redis redis_;
};

#endif