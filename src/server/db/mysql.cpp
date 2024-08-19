#include "mysql.hpp"

using namespace std;

static string host = "127.0.0.1";
static string user = "root";
static string passwd = "123456";
static string db = "chat";
static unsigned int port = 3306;

Mysql::Mysql()
{
    conn_ = mysql_init(nullptr);
    if (conn_ == nullptr)
    {
        LOG_ERROR << "fail to init mysql";
        exit(-1);
    }
}

Mysql::~Mysql()
{
    if (conn_ != nullptr)
    {
        mysql_close(conn_);
    }
}

bool Mysql::connect()
{
    if (nullptr == mysql_real_connect(conn_, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0))
    {
        LOG_ERROR << "fail to connect mysql";
        return false;
    }
    // 设置编码集
    mysql_set_character_set(conn_, "utf8mb4");
    return true;
}

bool Mysql::update(const std::string &sql)
{
    int n = mysql_query(conn_, sql.c_str());
    if (n)
    {
        LOG_ERROR << "operator mysql error for: " << mysql_error(conn_);
        return false;
    }
    LOG_INFO << "update success";
    return true;
}

/**
 * @attention 操作后需要释放资源
 */
MYSQL_RES *Mysql::query(const std::string &sql)
{
    int n = mysql_query(conn_, sql.c_str());
    if (n)
    {
        LOG_ERROR << "operator mysql error for: " << mysql_error(conn_);
        return nullptr;
    }
    LOG_INFO << "query success";
    return mysql_store_result(conn_);
}

MYSQL *Mysql::getConnection()
{
    return conn_;
}