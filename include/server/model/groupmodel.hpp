#ifndef GROUPMODEL_HPP
#define GROUPMODEL_HPP

#include <string>
#include <vector>

#include "group.hpp"
#include "groupuser.hpp"

/**
 * @brief 群组操作类
 */
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);

    // 添加群组
    bool addGroup(int groupid, int userid, const std::string &role = "normal");

    // 查询某个群组信息
    Group queryGroup(int groupid);

    // 查询用户所有群组信息
    std::vector<Group> queryGroups(int userid);

    // 查询群组群员信息(不包含userid)
    std::vector<GroupUser> queryGroupUsers(int groupid, int userid);
};

#endif