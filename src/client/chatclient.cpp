#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
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

// 获取当前时间
std::string getCurrentTime()
{
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);

    stringstream ss;

    ss << put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

// ============================================
// ChatClient类实现
ChatClient::ChatClient(const std::string &ip, u_int16_t port)
{
    this->ip = ip;
    this->port = port;
    Connect();

    // 接受线程启动
    thread t(&ChatClient::Recv, this);
    t.detach();
}

ChatClient::~ChatClient()
{
    close(fd);
}

void ChatClient::init()
{
    msgRecvHandlerMap_.emplace(LOGIN_MSG_ACK, bind(&ChatClient::loginRecv, this, _1));
    msgRecvHandlerMap_.emplace(REG_MSG_ACK, bind(&ChatClient::regRecv, this, _1));
    msgRecvHandlerMap_.emplace(P2P_CHAT_MSG, bind(&ChatClient::chatRecv, this, _1));
    msgRecvHandlerMap_.emplace(P2P_CHAT_MSG_ACK, bind(&ChatClient::chatackRecv, this, _1));
    msgRecvHandlerMap_.emplace(ADD_FRIEND_REQ_MSG, bind(&ChatClient::addfriendRecv, this, _1));
    msgRecvHandlerMap_.emplace(ADD_FRIEND_REQ_MSG_ACK, bind(&ChatClient::addfriendackRecv, this, _1));
    msgRecvHandlerMap_.emplace(ADD_FRIEND_VERIFY_MSG_ACK, bind(&ChatClient::verifyfriendackRecv, this, _1));
    msgRecvHandlerMap_.emplace(CREATE_GROUP_MSG_ACK, bind(&ChatClient::creategroupackRecv, this, _1));
    msgRecvHandlerMap_.emplace(ADD_GROUP_MSG_ACK, bind(&ChatClient::addgroupackRecv, this, _1));
    msgRecvHandlerMap_.emplace(GROUP_CHAT_MSG, bind(&ChatClient::groupchatRecv, this, _1));
    msgRecvHandlerMap_.emplace(REFRESH_MSG_ACK, bind(&ChatClient::refreshackRecv, this, _1));

    commandMap_.emplace("help", "显示支持的命令");
    commandMap_.emplace("show", "显示当前登录的用户信息");
    commandMap_.emplace("logout", "退出登录");
    commandMap_.emplace("chat", "点对点聊天chat:to:msg");
    commandMap_.emplace("addfriend", "发送添加好友请求addfriend:to");
    commandMap_.emplace("verifyfriend", "验证好友请求verifyfriend:to:[y/n]");
    commandMap_.emplace("creategroup", "创建群组creategroup:groupname:groupdesc");
    commandMap_.emplace("addgroup", "添加群组addgroup:groupid");
    commandMap_.emplace("groupchat", "群组聊天groupchat:groupid:msg");
    commandMap_.emplace("refresh", "刷新信息refresh");

    commandHandlerMap_.emplace("help", bind(&ChatClient::help, this, _1));
    commandHandlerMap_.emplace("show", bind(&ChatClient::show, this, _1));
    commandHandlerMap_.emplace("logout", bind(&ChatClient::logout, this, _1));
    commandHandlerMap_.emplace("chat", bind(&ChatClient::chat, this, _1));
    commandHandlerMap_.emplace("addfriend", bind(&ChatClient::addfriend, this, _1));
    commandHandlerMap_.emplace("verifyfriend", bind(&ChatClient::verifyfriend, this, _1));
    commandHandlerMap_.emplace("creategroup", bind(&ChatClient::creategroup, this, _1));
    commandHandlerMap_.emplace("addgroup", bind(&ChatClient::addgroup, this, _1));
    commandHandlerMap_.emplace("groupchat", bind(&ChatClient::groupchat, this, _1));
    commandHandlerMap_.emplace("refresh", bind(&ChatClient::refresh, this, _1));
}

// 连接函数
void ChatClient::Connect()
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

// 接受线程执行的函数
void ChatClient::Recv()
{
    // cout<<"Recv Start"<<endl;
    for (;;)
    {
        char buf[1024] = {0};
        int n = recv(fd, (void *)buf, sizeof(buf), 0);
        if (0 == n)
        {
            close(fd);
            exit(0);
        }
        if (-1 == n)
        {
            cerr << "recv fail" << endl;
            continue;
        }
        buf[n] = '\0';
        string str(buf);
        json js = json::parse(str);
        int msgid = js["msgid"].get<int>();
        auto it = msgRecvHandlerMap_.find(msgid);
        if (it != msgRecvHandlerMap_.end())
        {
            it->second(js);
        }
        else
        {
            cout << "Recv Type: " << msgid << endl;
        }
    }
}

// 主界面
void ChatClient::mainMenu()
{
    cout << "******主界面******" << endl;
    help();
    for (;;)
    {
        string line;
        getline(cin, line);
        auto it = line.find(":");
        if (it == string::npos)
        {
            // 无参的命令
            auto handler = commandHandlerMap_.find(line);
            if (handler == commandHandlerMap_.end())
            {
                cerr << "没有相关函数" << endl;
                continue;
            }
            handler->second("");
        }
        else
        {
            // 需要参数的命令
            auto handler = commandHandlerMap_.find(line.substr(0, it));
            if (handler == commandHandlerMap_.end())
            {
                cerr << "没有相关函数" << endl;
                continue;
            }
            handler->second(line.substr(it + 1));
        }
    }
}

void ChatClient::start()
{
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

// =======================================
// 命令函数
void ChatClient::help(const std::string &str)
{
    cout << "******命令列表******" << endl;
    for (auto it = commandMap_.begin(); it != commandMap_.end(); ++it)
    {
        cout << it->first << ":" << it->second << endl;
    }
}

void ChatClient::show(const std::string &str)
{
    cout << "******当前用户******" << endl;
    cout << user.getId() << " " << user.getUsername() << endl;
    if (!friends.empty())
    {
        cout << "******好友列表******" << endl;
        for (auto &user : friends)
        {
            cout << user.getId() << " " << user.getUsername() << " " << user.getState() << endl;
        }
    }
    if (!friends.empty())
    {
        cout << "******群组列表******" << endl;
        for (auto &group : groups)
        {
            cout << group.getId() << " " << group.getGroupname() << " " << group.getGroupdesc() << endl;
        }
    }
}

void ChatClient::logout(const std::string &str)
{
    close(fd);
    exit(0);
}

void ChatClient::chat(const std::string &str)
{
    auto it = str.find(":");
    if (it != string::npos)
    {
        json js;
        js["msgid"] = P2P_CHAT_MSG;
        js["id"] = user.getId();
        js["from"] = user.getUsername();
        js["to"] = stoi(str.substr(0, it));
        js["msg"] = str.substr(it + 1);
        js["time"] = getCurrentTime();

        string s = js.dump();

        if (-1 == send(fd, s.c_str(), s.size() + 1, 0))
        {
            cerr << "chat send fail" << endl;
        }
    }
}

void ChatClient::addfriend(const std::string &str)
{
    json js;
    js["msgid"] = ADD_FRIEND_REQ_MSG;
    js["id"] = user.getId();
    js["from"] = user.getUsername();
    js["to"] = stoi(str);
    js["time"] = getCurrentTime();

    string s = js.dump();

    if (-1 == send(fd, s.c_str(), s.size() + 1, 0))
    {
        cerr << "addfriend send fail" << endl;
    }
}

void ChatClient::verifyfriend(const std::string &str)
{
    auto it = str.find(":");
    if (it != string::npos)
    {
        json js;
        js["msgid"] = ADD_FRIEND_VERIFY_MSG;
        js["id"] = user.getId();
        js["from"] = user.getUsername();
        js["time"] = getCurrentTime();
        js["to"] = stoi(str.substr(0, it));
        js["agree"] = (bool)(str.substr(it + 1)[0] == 'y');

        string s = js.dump();

        if (-1 == send(fd, s.c_str(), s.size() + 1, 0))
        {
            cerr << "verifyfriend send fail" << endl;
        }
    }
}

void ChatClient::creategroup(const std::string &str)
{
    auto it = str.find(":");
    if (it != string::npos)
    {
        json js;
        js["msgid"] = CREATE_GROUP_MSG;
        js["id"] = user.getId();
        js["groupname"] = str.substr(0, it);
        js["groupdesc"] = str.substr(it + 1);

        string s = js.dump();

        if (-1 == send(fd, s.c_str(), s.size() + 1, 0))
        {
            cerr << "creategroup send fail" << endl;
        }
    }
}

void ChatClient::addgroup(const std::string &str)
{
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = user.getId();
    js["groupid"] = stoi(str);

    string s = js.dump();

    if (-1 == send(fd, s.c_str(), s.size() + 1, 0))
    {
        cerr << "addgroup send fail" << endl;
    }
}

void ChatClient::groupchat(const std::string &str)
{
    auto it = str.find(":");
    if (it != string::npos)
    {
        json js;
        js["msgid"] = GROUP_CHAT_MSG;
        js["id"] = user.getId();
        js["from"] = user.getUsername();
        js["groupid"] = stoi(str.substr(0, it));
        js["msg"] = str.substr(it + 1);
        js["time"] = getCurrentTime();

        string s = js.dump();

        if (-1 == send(fd, s.c_str(), s.size() + 1, 0))
        {
            cerr << "groupchat send fail" << endl;
        }
    }
}

void ChatClient::refresh(const std::string &str)
{
    json js;
    js["msgid"] = REFRESH_MSG;
    js["id"] = user.getId();

    string s = js.dump();

    if (-1 == send(fd, s.c_str(), s.size() + 1, 0))
    {
        cerr << "refresn send fail" << endl;
    }

    // 等待响应
    cout<<"wait ..."<<endl;
    sem.wait();
}

// =======================================
// 消息接受回调
void ChatClient::loginRecv(nlohmann::json &js)
{
    int err = js["errno"].get<int>();
    if (!err)
    {
        user.setId(js["id"].get<int>());
        user.setUsername(js["username"].get<string>());
        if (!js["friendunverifys"].is_null())
        {
            vector<int> vec = js["friendunverifys"].get<vector<int>>();
            cout << "******好友请求******" << endl;
            for (auto &id : vec)
            {
                cout << "Id: " << id << endl;
            }
        }
        if (!js["friends"].is_null())
        {
            vector<string> vec = js["friends"].get<vector<string>>();
            for (auto &str : vec)
            {
                json j = json::parse(str);
                friends.emplace_back(j["id"].get<int>(), j["username"].get<string>(), "******", j["state"].get<string>());
            }
        }
        if (!js["groups"].is_null())
        {

            vector<string> vec = js["groups"].get<vector<string>>();
            for (auto &str : vec)
            {
                json j = json::parse(str);
                groups.emplace_back(j["groupid"].get<int>(), j["groupname"].get<string>(), j["groupdesc"].get<string>());
            }
        }
        if (!js["offlinemessages"].is_null())
        {
            vector<string> vec = js["offlinemessages"].get<vector<string>>();
            for (auto &str : vec)
            {
                json j = json::parse(str);
                auto it = msgRecvHandlerMap_.find(j["msgid"].get<int>());
                if (it != msgRecvHandlerMap_.end())
                {
                    it->second(j);
                }
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

void ChatClient::chatRecv(nlohmann::json &js)
{
    int id = js["id"].get<int>();
    string time = js["time"].get<string>();
    string from = js["from"].get<string>();
    string msg = js["msg"].get<string>();
    cout << "Time: " << time << " From: " << from << " Id: " << id << endl;
    cout << "Said: " << msg << endl;
}

void ChatClient::chatackRecv(nlohmann::json &js)
{
    int err = js["errno"].get<int>();
    if (err)
    {
        cerr << js["errmsg"].get<string>() << endl;
    }
}

void ChatClient::addfriendRecv(nlohmann::json &js)
{
    int id = js["id"].get<int>();
    string time = js["time"].get<string>();
    string from = js["from"].get<string>();
    string desc = js["desc"].get<string>();
    cout << "Time: " << time << " From: " << from << " Id: " << id << endl;
    cout << "Desc: " << desc << endl;
}

void ChatClient::addfriendackRecv(nlohmann::json &js)
{
    int err = js["errno"].get<int>();
    if (err)
    {
        cerr << js["errmsg"].get<string>() << endl;
    }
}

void ChatClient::verifyfriendackRecv(nlohmann::json &js)
{
    cout << js["desc"].get<string>() << endl;
}

void ChatClient::creategroupackRecv(nlohmann::json &js)
{
    int err = js["errno"].get<int>();
    if (!err)
    {
        cout << "创建群组[" << js["groupid"].get<int>() << "]" << endl;
    }
    else
    {
        cerr << js["errmsg"].get<string>() << endl;
    }
}

void ChatClient::addgroupackRecv(nlohmann::json &js)
{
    int err = js["errno"].get<int>();
    if (!err)
    {
        cout << js["desc"].get<string>() << endl;
    }
    else
    {
        cerr << js["errmsg"].get<string>() << endl;
    }
}

void ChatClient::groupchatRecv(nlohmann::json &js)
{
    int id = js["id"].get<int>();
    string from = js["from"].get<string>();
    int groupid = js["groupid"].get<int>();
    string time = js["time"].get<string>();
    string msg = js["msg"].get<string>();
    cout << "Groupid: " << groupid << endl;
    cout << "Time: " << time << " From: " << from << " Id: " << id << endl;
    cout << "Said: " << msg << endl;
}

void ChatClient::refreshackRecv(nlohmann::json &js)
{
    if(!js["user"].is_null())
    {
        json j=json::parse(js["user"].get<string>());
        user.setUsername(j["username"].get<string>());
        user.setState(j["state"].get<string>());
    }
    if (!js["friends"].is_null())
    {
        friends.clear();
        vector<string> vec = js["friends"].get<vector<string>>();
        for (auto &str : vec)
        {
            json j = json::parse(str);
            friends.emplace_back(j["id"].get<int>(), j["username"].get<string>(), "******", j["state"].get<string>());
        }
    }
    if (!js["groups"].is_null())
    {
        groups.clear();
        vector<string> vec = js["groups"].get<vector<string>>();
        for (auto &str : vec)
        {
            json j = json::parse(str);
            groups.emplace_back(j["groupid"].get<int>(), j["groupname"].get<string>(), j["groupdesc"].get<string>());
        }
    }

    //通知
    sem.post();
}