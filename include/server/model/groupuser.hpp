#ifndef GROUPUSER_HPP
#define GROUPUSER_HPP

#include <string>

#include "user.hpp"

/**
 * @brief 群组用户映射类
 */
class GroupUser : public User
{
public:
    GroupUser(int id = -1,
              const std::string &username = "",
              const std::string &password = "",
              const std::string &state = "offline",
              const std::string &role = "normal")
        : User(id, username, password, state), role_(role)
    {
    }

    std::string getRole()
    {
        return role_;
    }

    void setRole(const std::string &role)
    {
        role_ = role;
    }

protected:
    std::string role_;
};

#endif