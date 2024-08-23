#include <iostream>
#include <functional>
#include <signal.h>

#include "chatserver.hpp"
#include "usermodel.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;

void errsig_handler(int sig)
{
    UserModel userModel;
    userModel.updateAllState("offline");
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stdout, "Usage:%s ip port", argv[0]);
        exit(-1);
    }
    // 处理异常信号
    int signals[] = {SIGINT, SIGILL, SIGABRT, SIGBUS, SIGFPE, SIGKILL, SIGSEGV};

    for (int i = 0; i < sizeof(signals) / sizeof(signals[0]); i++)
    {
        signal(signals[i], errsig_handler);
    }

    EventLoop loop;
    InetAddress listenAddr(argv[1], atoi(argv[2]));

    ChatServer server(&loop, listenAddr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}