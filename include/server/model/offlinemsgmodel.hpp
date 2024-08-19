#ifndef OFFLINEMSGMODEL_HPP
#define OFFLINEMSGMODEL_HPP

#include <string>
#include <vector>

/**
 * @brief 离线消息表操作类
 */
class OfflineMsgModel
{
public:
    // 离线消息表新增方法
    bool insert(int id,const std::string& str);

    // 离线消息表删除方法
    bool erase(int id);

    // 离线消息表查询方法
    std::vector<std::string> query(int id);
};

#endif