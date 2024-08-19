#ifndef USERMODEL_HPP
#define USERMODEL_HPP

#include <string>

#include "user.hpp"

/**
 * @brief 用户表操作类
 */
class UserModel
{
public:
    // 用户表增加方法
    bool insert(User& user);

    // 用户表查询方法
    User query(int id);

    // 更新某个用户在线状态
    bool updateState(User& user);

    // 更新全体用户在线状态
    bool updateAllState(const std::string& state);
};

#endif