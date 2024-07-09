#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <iostream>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <cstdlib> // 使用 <cstdlib> 替代 <stdlib.h>，因为这是 C++ 标准库中的头文件
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h> // TCP_NODELAY需要包含这个头文件。
#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Connection.h"
#include <map>

class TcpServer
{
private:
    EventLoop loop_;
    Acceptor *acceptor_;                // 一个TcpServer只能有一个acceptor对象
    std::map<int, Connection *> conns_; // 一个TcpServer有多个Connection对象

public:
    TcpServer(const std::string &ip, uint16_t port);
    ~TcpServer();

    void start();

    void newconnection(Socket *clientsock);
    void closeconnection(Connection *conn); // 关闭客户端连接，回调函数
    void errorconnection(Connection *conn); 
};

#endif