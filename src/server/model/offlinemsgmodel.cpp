#include "offlinemsgmodel.hpp"
#include "mysql.hpp"

using namespace std;

// 离线消息表新增方法
bool OfflineMsgModel::insert(int id, const std::string &str)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessages(userid,offlinemessage) values(%d,'%s')",
            id, str.c_str());

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }

    return false;
}

// 离线消息表删除方法
bool OfflineMsgModel::erase(int id)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessages where userid=%d", id);

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }

    return false;
}

// 离线消息表查询方法
vector<string> OfflineMsgModel::query(int id)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "select offlinemessage from offlinemessages where userid=%d", id);

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            vector<string> vec;
            MYSQL_ROW row;
            while (nullptr != (row = mysql_fetch_row(res)))
            {
                vec.emplace_back(row[0]);
            }
            // 释放资源
            mysql_free_result(res);

            return vec;
        }
    }

    return {};
}