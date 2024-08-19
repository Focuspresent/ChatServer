#include <iostream>

#include "usermodel.hpp"
#include "mysql.hpp"

using namespace std;

// 用户表增加方法
bool UserModel::insert(User &user)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into users(username,password,state) values('%s','%s','%s')",
            user.getUsername().c_str(), user.getPassword().c_str(), user.getState().c_str());

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

// 用户表查询方法
User UserModel::query(int id)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "select id,username,password,state from users where id=%d", id);

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            User user;
            user.setId(atoi(row[0]));
            user.setUsername(row[1]);
            user.setPassword(row[2]);
            user.setState(row[3]);
            // 释放资源
            mysql_free_result(res);
            return user;
        }
    }

    // 操作失败
    return User();
}

// 更新用户在线状态
bool UserModel::updateState(User& user)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "update users set state='%s' where id=%d",
            user.getState().c_str(), user.getId());

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }

    return false;
}

// 更新全体用户在线状态
bool UserModel::updateAllState(const std::string& state)
{
    // 格式化sql语句
    char sql[1024]={0};
    sprintf(sql,"update users set state='%s'",state.c_str());

    Mysql mysql;
    if(mysql.connect()){
        return mysql.update(sql);
    }

    return false;
}