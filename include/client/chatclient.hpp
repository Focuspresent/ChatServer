#ifndef CHATCLIENT_HPP
#define CHATCLIENT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <atomic>

#include "user.hpp"
#include "semaphore.hpp"
#include "group.hpp"
#include "common.hpp"
#include "json.hpp"

// 获取当前时间
std::string getCurrentTime();

/**
 * @brief 客户端类
 */
class ChatClient
{
public:
    ChatClient(const std::string &ip, u_int16_t port);
    ~ChatClient();

    void init();

    void start();

protected:
    // 连接函数
    void Connect();

    // 接受线程执行的函数
    void Recv();

    // 主界面
    void mainMenu();

    // 接受消息的回调函数
    void loginRecv(nlohmann::json& js);
    void regRecv(nlohmann::json& js);
    void chatRecv(nlohmann::json& js);
    void chatackRecv(nlohmann::json& js);
    void addfriendRecv(nlohmann::json& js);
    void addfriendackRecv(nlohmann::json& js);
    void verifyfriendackRecv(nlohmann::json& js);
    void creategroupackRecv(nlohmann::json& js);
    void addgroupackRecv(nlohmann::json& js);

    // 命令函数
    void help(const std::string& str="");
    void show(const std::string& str="");
    void logout(const std::string& str="");
    void chat(const std::string& str);
    void addfriend(const std::string& str);
    void verifyfriend(const std::string& str);
    void creategroup(const std::string& str);
    void addgroup(const std::string& str);

private:
    std::string ip; ///< 服务器地址
    u_int16_t port; ///< 服务器端口

    int fd;                    ///< 套接字
    User user;                 ///< 当前登录的用户信息
    std::vector<User> friends; ///< 好友信息
    std::vector<Group> groups; ///< 群组信息

    bool checkLogin=false; ///< 验证登录

    // 线程通信
    Semaphore<> sem;

    // 消息接受回调
    std::unordered_map<int,std::function<void(nlohmann::json& js)>> msgRecvHandlerMap_;

    // 命令显示
    std::unordered_map<std::string,std::string> commandMap_;

    // 命令发送回调
    std::unordered_map<std::string,std::function<void(const std::string&)>> commandHandlerMap_;
};

#endif