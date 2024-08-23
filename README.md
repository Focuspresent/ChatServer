集群聊天服务器
=======================
### 1.简介
基于muduo网络库实现的聊天服务器，使用nginx的tcp负载均衡模块以及redis的pub/sub模式，还有一个多线程客户端  
1. 基于高性能muduo网络库实现的服务端
2. 采用json作为信息传输协议
3. 使用mysql作为数据库
4. 配置nginx的tcp负载均衡模块实现集群
5. 基于redis的pub/sub模式实现跨服务器
6. 多线程客户端，读写分离
### 2.编译
```shell
chmod a+x autobuild.sh
./autobuild.sh
```
### 3. 使用
```shell
./bin/ChatServer [port]
./bin/ChatClient [ip] [port]
```