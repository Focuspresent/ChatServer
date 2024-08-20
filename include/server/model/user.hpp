#ifndef USER_HPP
#define USER_HPP

#include <string>

/**
 * @brief 用户表映射类
 */
class User
{
public:
    User(int id = -1,
         const std::string &username = "",
         const std::string &password = "",
         const std::string &state = "offline")
        : id_(id), username_(username), password_(password), state_(state)
    {
    }

    void setId(int id)
    {
        id_ = id;
    }

    void setUsername(const std::string &username)
    {
        username_ = username;
    }

    void setPassword(const std::string &password)
    {
        password_ = password;
    }

    void setState(const std::string &state)
    {
        state_ = state;
    }

    int getId()
    {
        return id_;
    }

    std::string getUsername()
    {
        return username_;
    }

    std::string getPassword()
    {
        return password_;
    }

    std::string getState()
    {
        return state_;
    }

protected:
    int id_;
    std::string username_;
    std::string password_;
    std::string state_;
};

#endif