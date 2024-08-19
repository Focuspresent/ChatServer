#ifndef MYSQL_HPP
#define MYSQL_HPP

#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
#include <string>
#include <vector>

/**
 * @brief 数据库操作类
 */
class Mysql
{
public:
    Mysql();
    ~Mysql();

    bool connect();
    bool update(const std::string &sql);
    bool transactionHandle(const std::vector<const char *> &sqls);
    MYSQL_RES *query(const std::string &sql);
    MYSQL *getConnection();

    Mysql(const Mysql &) = delete;
    Mysql &operator=(const Mysql &) = delete;
    Mysql(Mysql &&) = delete;
    Mysql &operator=(Mysql &&) = delete;

private:
    MYSQL *conn_ = nullptr;
};

#endif