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

int main()
{
    // 处理异常信号
    int signals[] = {SIGINT, SIGILL, SIGABRT, SIGBUS, SIGFPE, SIGKILL, SIGSEGV};

    for (int i = 0; i < sizeof(signals) / sizeof(signals[0]); i++)
    {
        signal(signals[i], errsig_handler);
    }

    EventLoop loop;
    InetAddress listenAddr("127.0.0.1", 9000);

    ChatServer server(&loop, listenAddr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}