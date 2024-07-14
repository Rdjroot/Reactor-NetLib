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
    std::function<void(Connection*)> newconnectioncb_;          // 回调EchoServer::HandleNewConnection()。
    std::function<void(Connection*)> closeconnectioncb_;        // 回调EchoServer::HandleClose()。
    std::function<void(Connection*)> errorconnectioncb_;         // 回调EchoServer::HandleError()。
    std::function<void(Connection*,std::string &message)> onmessagecb_;        // 回调EchoServer::HandleMessage()。
    std::function<void(Connection*)> sendcompletecb_;            // 回调EchoServer::HandleSendComplete()。
    std::function<void(EventLoop*)>  timeoutcb_;                       // 回调EchoServer::HandleTimeOut()。

public:
    TcpServer(const std::string &ip, uint16_t port);
    ~TcpServer();

    void start();

    void newconnection(Socket *clientsock);     // 处理新的客户端连接，回调函数
    void closeconnection(Connection *conn);     // 关闭客户端连接，回调函数
    void errorconnection(Connection *conn);     // 客户端连接出错关闭，回调函数
    void onmessage(Connection *conn, std::string &message);      // 处理客户端的请求报文
    void sendcomplete(Connection *conn);
    void epolltimeout(EventLoop *loop);         // epoll_wait()超时回调函数
    
    void setnewconnectioncb(std::function<void(Connection*)> fn);
    void setcloseconnectioncb(std::function<void(Connection*)> fn);
    void seterrorconnectioncb(std::function<void(Connection*)> fn);
    void setonmessagecb(std::function<void(Connection*,std::string &message)> fn);
    void setsendcompletecb(std::function<void(Connection*)> fn);
    void settimeoutcb(std::function<void(EventLoop*)> fn);

};

#endif