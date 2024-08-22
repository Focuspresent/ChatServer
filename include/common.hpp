#ifndef COMMON_HPP
#define COMMON_HPP

// 消息类型枚举
enum MsgType
{
    LOGIN_MSG=1, //登录消息
    LOGIN_MSG_ACK=2, //登录响应消息

    REG_MSG=3, //注册消息
    REG_MSG_ACK=4, //注册响应消息

    P2P_CHAT_MSG=5, // 点对点消息
    P2P_CHAT_MSG_ACK=6, // 点对点响应消息

    ADD_FRIEND_REQ_MSG=7, //添加好友请求消息
    ADD_FRIEND_REQ_MSG_ACK=8, //添加好友请求响应消息
    ADD_FRIEND_VERIFY_MSG=9, //添加好友验证消息
    ADD_FRIEND_VERIFY_MSG_ACK=10, //添加好友验证响应消息

    CREATE_GROUP_MSG=11, //创建群组消息
    CREATE_GROUP_MSG_ACK=12, //创建群组响应消息
    ADD_GROUP_MSG=13, //添加群组消息
    ADD_GROUP_MSG_ACK=14, //添加群组响应消息
    GROUP_CHAT_MSG=15, //群组聊天消息

    REFRESH_MSG=16, //刷新消息
    REFRESH_MSG_ACK=17, //刷新响应消息
};

#endif