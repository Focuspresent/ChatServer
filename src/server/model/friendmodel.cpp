#include <iostream>

#include "friendmodel.hpp"
#include "mysql.hpp"

using namespace std;

// 新增一条记录
bool FriendModel::insert(int userid, int friendid, const std::string &state)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friends(userid,friendid,state) values(%d,%d,'%s')",
            userid, friendid, state.c_str());

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }

    return false;
}

// 添加好友请求验证通过
bool FriendModel::pass(int userid, int friendid)
{
    // 格式化sql语句
    vector<string> vec;
    char sql[1024] = {0};
    sprintf(sql, "update friends set state='pass' where userid=%d and friendid=%d",
            userid, friendid);
    vec.emplace_back(sql);

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "insert into friends(userid,friendid,state) values(%d,%d,'%s')",
            friendid, userid, "pass");
    vec.emplace_back(sql);

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        return mysql.transactionHandle(vec);
    }

    return false;
}

// 添加好友请求验证不通过
bool FriendModel::unpass(int userid, int friendid)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from friends where userid=%d and friendid=%d",
            userid, friendid);

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }

    return false;
}

// 查询好友信息
std::vector<User> FriendModel::queryFriends(int userid)
{
    std::vector<User> vec;
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.username,a.state from users a inner join friends b on a.id=b.friendid where b.userid=%d", userid);

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while (nullptr != (row = mysql_fetch_row(res)))
            {
                vec.emplace_back(atoi(row[0]), row[1], "*****", row[2]);
            }
            // 释放资源
            mysql_free_result(res);
        }
    }

    return vec;
}

// 查询未确认的请求
std::vector<int> FriendModel::queryUnverify(int userid)
{
    std::vector<int> vec;
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "select friendid from friends where userid=%d and state='unverify'", userid);

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while (nullptr != (row = mysql_fetch_row(res)))
            {
                vec.emplace_back(atoi(row[0]));
            }
            // 释放资源
            mysql_free_result(res);
        }
    }

    return vec;
}