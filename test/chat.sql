--创建用户表--
CREATE TABLE `users`(
    `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '用户id',
    `username` VARCHAR(32) UNIQUE NOT NULL COMMENT '用户名',
    `password` VARCHAR(32) NOT NULL COMMENT '用户密码',
    `state` ENUM('offline','online') DEFAULT 'offline' COMMENT '是否在线'
);

--创建好友表--
CREATE TABLE `friends`(
    `userid` INT NOT NULL COMMENT '用户id',
    `friendid` INT NOT NULL COMMENT '好友id',
    `state` ENUM('pass','unpass','unclick') DEFAULT 'unclick' COMMENT '当前状态',
    PRIMARY KEY (`userid`,`friendid`)
)

--创建离线消息表--
CREATE TABLE `offlinemessages`(
    `userid` INT NOT NULL COMMENT '用户id',
    `offlinemessage` VARCHAR(512) NOT NULL COMMENT '离线的消息',
    KEY `userid_idx` (`userid`)
);

--创建群组表--
CREATE TABLE `groups`(
    `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '群组id',
    `groupname` VARCHAR(64) UNIQUE NOT NULL COMMENT '群组名',
    `groupdesc` VARCHAR(128) COMMENT '群组描述'
);

--创建群组用户表--
CREATE TABLE `groupusers`(
    `groupid` INT NOT NULL COMMENT '群组id',
    `userid` INT NOT NULL COMMENT '用户id',
    `role` ENUM('owner','admin','normal') DEFAULT 'normal' COMMENT '用户在群组中的角色',
    PRIMARY KEY (`groupid`,`userid`)
);