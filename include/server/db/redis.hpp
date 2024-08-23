#ifndef REDIS_HPP
#define REDIS_HPP

#include <hiredis/hiredis.h>
#include <string>
#include <functional>

/**
 * @brief redis pub/sub 实现类消息队列
 */
class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器
    bool connect();

    // 向通道发送消息
    bool publish(int channel,const std::string& message);

    // 订阅通道
    bool subscribe(int channel);

    // 取消订阅通道
    bool unsubscribe(int channel);

    // 设置回调函数
    void setsubCallBack(std::function<void(int,const std::string&)> cb);

    // 订阅接受执行的函数
    void subRecv();
private:
    redisContext* sub_; ///< 订阅
    redisContext* pub_; ///< 发布

    std::function<void(int,const std::string&)> subCallBack_;
};

#endif