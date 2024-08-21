#include <iostream>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

#include "chatclient.hpp"

using namespace std;
using namespace std::placeholders;
using json = nlohmann::json;

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

void ChatClient::init()
{
    msgRecvHandlerMap_.emplace(LOGIN_MSG_ACK, bind(&ChatClient::loginRecv, this, _1));
    msgRecvHandlerMap_.emplace(REG_MSG_ACK, bind(&ChatClient::regRecv, this, _1));
}

// 接受线程执行的函数
void ChatClient::Recv()
{
    for (;;)
    {
        char buf[1024] = {0};
        int n = recv(fd, (void *)buf, sizeof(buf), 0);
        if (-1 == n)
        {
            cerr << "recv fail" << endl;
            continue;
        }
        buf[n] = '\0';
        string str(buf);
        json js = json::parse(str);
        auto it = msgRecvHandlerMap_.find(js["msgid"].get<int>());
        if (it != msgRecvHandlerMap_.end())
        {
            it->second(js);
        }
    }
}

// 主界面
void ChatClient::mainMenu()
{
    cout << "******主界面******" << endl;
    if(!friends.empty())
    {
        cout << "******好友列表******" << endl;
        for(auto& user: friends)
        {
            cout<<user.getId()<<" "<<user.getUsername()<<" "<<user.getState()<<endl;
        }
    }
    if(!friends.empty())
    {
        cout << "******群组列表******" << endl;
        for(auto& group: groups)
        {
            cout<<group.getId()<<" "<<group.getGroupname()<<" "<<group.getGroupdesc()<<endl;
        }
    }
    for (;;)
    {
    }
}

void ChatClient::start()
{
    // 接受线程启动
    thread t(&ChatClient::Recv, this);
    t.detach();

    for (;;)
    {
        cout << "*****************************" << endl;
        cout << "\t" << "login: 1" << "\t" << endl;
        cout << "\t" << "register: 2" << "\t" << endl;
        cout << "\t" << "exit: 3" << "\t" << endl;
        cout << "*****************************" << endl;
        int choice;
        cin >> choice;
        cin.get();
        switch (choice)
        {
        case 1:
        {
            // 读取登录信息
            int id;
            string password;
            cout << "请输入账号:" << endl;
            cin >> id;
            cin.get();
            cout << "请输入密码:" << endl;
            getline(cin, password);

            // 序列化
            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = password;
            string str = js.dump();

            // 发送
            int n = send(fd, str.c_str(), str.size() + 1, 0);
            if (-1 == n)
            {
                // 写入失败
                cerr << "send fail" << endl;
                continue;
            }

            // 等待响应
            sem.wait();
            if (checkLogin)
            {
                // 进入主界面
                mainMenu();
            }
            else
            {
                continue;
            }
        }
        break;
        case 2:
        {
            // 读取注册信息
            string username, password;
            cout << "请输入账号名:" << endl;
            getline(cin, username);
            cout << "请输入密码:" << endl;
            getline(cin, password);

            // 序列化
            json js;
            js["msgid"] = REG_MSG;
            js["username"] = username;
            js["password"] = password;
            string str = js.dump();

            // 发送
            int n = send(fd, str.c_str(), str.size() + 1, 0);
            if (-1 == n)
            {
                // 写入失败
                cerr << "send fail" << endl;
                continue;
            }

            // 等待响应
            sem.wait();
        }
        break;
        case 3:
            close(fd);
            exit(0);
            break;
        default:
            cout << "无效选择" << endl;
            break;
        }
    }
}

void ChatClient::loginRecv(nlohmann::json &js)
{
    int err = js["errno"].get<int>();
    if (!err)
    {
        user.setId(js["id"].get<int>());
        user.setUsername(js["username"].get<string>());
        if(!js["friends"].is_null())
        {   
            vector<string> vec=js["friends"].get<vector<string>>();
            for(auto& str: vec)
            {
                json j=json::parse(str);
                friends.emplace_back(j["id"].get<int>(),j["username"].get<string>(),"******",j["state"].get<string>());
            }
        }
        if(!js["groups"].is_null())
        {

            vector<string> vec=js["groups"].get<vector<string>>();
            for(auto& str: vec)
            {
                json j=json::parse(str);
                groups.emplace_back(j["groupid"].get<int>(),j["groupname"].get<string>(),j["groupdesc"].get<string>());
            }
        }
        cout << "登录成功" << endl;
        checkLogin = true;
    }
    else
    {
        cerr << js["errmsg"].get<string>() << endl;
    }
    sem.post();
}

void ChatClient::regRecv(nlohmann::json &js)
{
    int err = js["errno"].get<int>();
    if (!err)
    {
        int id = js["id"].get<int>();
        cout << "请记住[" << id << "]" << endl;
    }
    else
    {
        cerr << js["errmsg"].get<string>() << endl;
    }
    sem.post();
}