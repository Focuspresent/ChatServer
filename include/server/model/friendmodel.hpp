#ifndef FRIENDMODEL_HPP
#define FRIENDMODEL_HPP

#include <string>
#include <vector>

#include "user.hpp"

/**
 * @brief 好友表操作类
 */
class FriendModel
{
public:
    // 新增一条记录
    bool insert(int userid, int friendid, const std::string &state = "unverify");

    // 添加好友请求验证通过
    bool pass(int userid, int friendid);

    // 添加好友请求验证不通过
    bool unpass(int userid,int friendid);

    // 查询好友信息
    std::vector<User> queryFriends(int userid);

    // 查询未确认的请求
    std::vector<int> queryUnverify(int userid);
};

#endif