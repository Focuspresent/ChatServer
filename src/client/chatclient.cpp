#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

#include "chatclient.hpp"

using namespace std;

ChatClient::ChatClient(const std::string &ip, u_int16_t port)
{
    // 创建套接字
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
    {
        cerr << "socket fail" << " " << strerror(errno) << endl;
        exit(-1);
    }

    // 填充远端地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    // 连接
    if (-1 == connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        cerr << "connect fail" << " " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }
}

ChatClient::~ChatClient()
{
    close(fd);
}

void ChatClient::start()
{

}