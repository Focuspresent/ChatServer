#include <functional>

#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

using namespace std;
using namespace std::placeholders;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

ChatServer::ChatServer(muduo::net::EventLoop *loop,
                       const muduo::net::InetAddress &listenAddr,
                       const std::string &nameArg)
    : loop_(loop), server_(loop, listenAddr, nameArg)
{
    // 注册回调函数
    server_.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

    server_.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

    server_.setThreadNum(4);
}

void ChatServer::start()
{
    server_.start();
}

void ChatServer::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 客户端下线
        ChatService::getInstance()->clientLogout(conn);

        // 关闭链接
        conn->shutdown();
    }
}

void ChatServer::onMessage(const muduo::net::TcpConnectionPtr &conn,
                           muduo::net::Buffer *buf,
                           muduo::Timestamp time)
{
    string str = buf->retrieveAllAsString();
    json js = json::parse(str);
    ChatService::getInstance()->getHandler(js["msgid"].get<int>())(conn, js, time);
}