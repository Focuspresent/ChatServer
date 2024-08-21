#ifndef CHATCLIENT_HPP
#define CHATCLIENT_HPP

#include <string>

/**
 * @brief 客户端类
 */
class ChatClient
{
public:
    ChatClient(const std::string& ip,u_int16_t port);
    ~ChatClient();

    void start();

private:
    int fd; ///< 套接字
};

#endif 