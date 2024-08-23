#include <iostream>
#include <thread>

#include "redis.hpp"

using namespace std;

Redis::Redis()
    : sub_(nullptr), pub_(nullptr)
{
}

Redis::~Redis()
{
    if (nullptr != sub_)
    {
        redisFree(sub_);
    }
    if (nullptr != pub_)
    {
        redisFree(pub_);
    }
}

// 连接redis服务器
bool Redis::connect()
{
    sub_ = redisConnect("127.0.0.1", 6379);
    if (nullptr == sub_ || sub_->err)
    {
        if (sub_)
        {
            cerr << sub_->errstr << endl;
        }
        else
            cerr << "connect redis fail" << endl;
        return false;
    }
    pub_ = redisConnect("127.0.0.1", 6379);
    if (nullptr == pub_ || pub_->err)
    {
        if (pub_)
        {
            cerr << pub_->errstr << endl;
        }
        else
            cerr << "connect redis fail" << endl;
        return false;
    }

    thread t([&]()
             { subRecv(); });
    t.detach();

    return true;
}

// 向通道发送消息
bool Redis::publish(int channel, const std::string &message)
{
    redisReply *reply = (redisReply *)redisCommand(pub_, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply)
    {
        cerr << "publish command fail" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 订阅通道
bool Redis::subscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(sub_, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command fail" << endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(sub_, &done))
        {
            cerr << "subscribe commadn fail" << endl;
            return false;
        }
    }
    return true;
}

// 取消订阅通道
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(sub_, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command fail" << endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(sub_, &done))
        {
            cerr << "unsubscribe commadn fail" << endl;
            return false;
        }
    }
    return true;
}

// 设置回调函数
void Redis::setsubCallBack(std::function<void(int, const std::string &)> cb)
{
    subCallBack_ = cb;
}

// 订阅接受执行的函数
void Redis::subRecv()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(sub_, (void **)&reply))
    {
        if (nullptr != reply && nullptr != reply->element[2] && nullptr != reply->element[2]->str)
        {
            subCallBack_(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    cerr << "subRecv fail" << endl;
}