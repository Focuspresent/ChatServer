#include <iostream>

#include "groupmodel.hpp"
#include "mysql.hpp"

using namespace std;

// 创建群组
bool GroupModel::createGroup(Group &group)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into `groups`(groupname,groupdesc) values('%s','%s')",
            group.getGroupname().c_str(), group.getGroupdesc().c_str());

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

// 添加群组
bool GroupModel::addGroup(int groupid, int userid, const std::string &role)
{
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into groupusers(groupid,userid,role) values(%d,%d,'%s')",
            groupid, userid, role.c_str());

    // 操作数据库
    Mysql mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }

    return false;
}

// 查询某个群组信息
Group GroupModel::queryGroup(int groupid)
{
    // 格式化
    char sql[1024]={0};
    sprintf(sql,"select groupname,groupdesc from `groups` where id=%d",groupid);

    // 操作数据库
    Mysql mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);

            // 释放资源
            mysql_free_result(res);

            return Group(groupid,row[0],row[1]);
        }
    }

    return Group();
}

// 查询群组信息
std::vector<Group> GroupModel::queryGroups(int userid)
{
    vector<Group> vec;

    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from `groups` a inner join groupusers b on a.id=b.groupid where b.userid=%d",userid);

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
                vec.emplace_back(atoi(row[0]), row[1], row[2]);
            }
            // 释放资源
            mysql_free_result(res);
        }
    }

    return vec;
}

// 查询群组群员信息(不包含userid)
std::vector<GroupUser> GroupModel::queryGroupUsers(int groupid, int userid)
{
    vector<GroupUser> vec;
    // 格式化sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.username,a.state,b.role from users a inner join groupusers b on a.id=b.userid where b.groupid=%d", groupid);

    // 操作数据库
    Mysql mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while(nullptr!=(row=mysql_fetch_row(res)))
            {
                if(atoi(row[0])==userid) continue;
                vec.emplace_back(atoi(row[0]),row[1],"*****",row[2],row[3]);
            }
            // 释放资源
            mysql_free_result(res);
        }
    }

    return vec;
}