#ifndef CHATCLIENT_HPP
#define CHATCLIENT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "user.hpp"
#include "semaphore.hpp"
#include "group.hpp"
#include "common.hpp"
#include "json.hpp"

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
    // 接受线程执行的函数
    void Recv();

    // 主界面
    void mainMenu();

    // 接受消息的回调函数
    void loginRecv(nlohmann::json& js);
    void regRecv(nlohmann::json& js);

private:
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

    // 命令发送回调
};

#endif