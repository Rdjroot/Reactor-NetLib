#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "Logger.h"
#include <unistd.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include<atomic>

extern Logger &logger;

class TcpServer
{
private:
    std::unique_ptr<EventLoop> mainloop_;              // 主事件循环
    std::vector<std::unique_ptr<EventLoop>> subloops_; // 从事件循环容器
    Acceptor acceptor_;                                // 一个TcpServer只能有一个acceptor对象(服务端)
    int threadnum_;                                    // 线程池大小
    ThreadPool threadpool_;                            // 线程池
    std::mutex mutex_;                                 // conns_的互斥锁
    std::unordered_map<int, spConnection> conns_;      // 一个TcpServer有多个Connection对象

    std::atomic<long long> recvInfo;

    std::function<void(spConnection)> newconnectioncb_;                   // 回调EchoServer::HandleNewConnection()。
    std::function<void(spConnection)> closeconnectioncb_;                 // 回调EchoServer::HandleClose()。
    std::function<void(spConnection)> errorconnectioncb_;                 // 回调EchoServer::HandleError()。
    std::function<void(spConnection, std::string &message)> onmessagecb_; // 回调EchoServer::HandleMessage()。
    std::function<void(spConnection)> sendcompletecb_;                    // 回调EchoServer::HandleSendComplete()。
    std::function<void(EventLoop *)> timeoutcb_;                          // 回调EchoServer::HandleTimeOut()。

public:
    TcpServer(const std::string &ip, uint16_t port, int threadnum);
    ~TcpServer();

    void start(); // 开启事件循环
    void stop();  // 停止IO线程和事件循环

    void newconnection(std::unique_ptr<Socket> clientsock); // 处理新的客户端连接，回调函数
    void closeconnection(spConnection conn);                // 关闭客户端连接，回调函数
    void errorconnection(spConnection conn);                // 客户端连接出错关闭，回调函数
    void epolltimeout(EventLoop *loop);                     // epoll_wait()超时回调函数

    void onmessage(spConnection conn, std::string &message); // 处理客户端的请求报文
    void sendcomplete(spConnection conn);                    // 回复客户端报文

    void removeconn(int fd); // 删除conns_中的Connection对象，在事件循环的超时判断里回调

    // 设置与应用层对接的回调函数
    void setnewconnectioncb(std::function<void(spConnection)> fn);
    void setcloseconnectioncb(std::function<void(spConnection)> fn);
    void seterrorconnectioncb(std::function<void(spConnection)> fn);
    void setonmessagecb(std::function<void(spConnection, std::string &message)> fn);
    void setsendcompletecb(std::function<void(spConnection)> fn);
    void settimeoutcb(std::function<void(EventLoop *)> fn);
};

#endif