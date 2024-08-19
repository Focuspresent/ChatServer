#ifndef COMMON_HPP
#define COMMON_HPP

// 消息类型枚举
enum MsgType
{
    LOGIN_MSG=1, //登录消息
    LOGIN_MSG_ACK, //登录响应消息
    REG_MSG, //注册消息
    REG_MSG_ACK, //注册响应消息
    P2P_MSG, // 点对点消息
    P2P_MSG_ACK // 点对点响应消息
};

#endif