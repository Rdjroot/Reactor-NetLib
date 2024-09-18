#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
#include <atomic>
#include <memory>
#include <sys/syscall.h>
#include "Timestamp.h"
#include "Logger.h"
#include<thread>

class EventLoop;
class Channel;
class Connection;
extern Logger &logger;
using spConnection = std::shared_ptr<Connection>;

/**
 * Connection类：封装了一个已建立的TCP连接，并进行管理
 * 
 * 功能：控制（建立、关闭、销毁）连接，所监听事件的处理方式，套接字信息，时间信息
 * [一个Connection对应一个EventLoop（Epoll），一个线程池，属于Sub Reactor里的]
 * 
*/
class Connection : public std::enable_shared_from_this<Connection>
{
private:
    EventLoop *loop_;                        // Connection对应的事件循环，从构造函数传入，使用但不拥有
    std::unique_ptr<Socket> clientsock_;     // 与客户端通信的socket(外界传入，但管理其生命周期)
    std::unique_ptr<Channel> clientchannel_; // 对应的Channel，在构造函数中创建（拥有）
    Buffer inputbuffer_;                     // 接收缓冲区
    Buffer outputbuffer_;                    // 发送缓冲区
    std::atomic_bool disconnect_;            // 客户端是否已断开，如果已断开，设为true
    Timestamp lastime_;                      // 时间戳，创建 Connection时为当前时间，每接收到一个报文，把时间戳更新为当前时间

    bool flag_;

    std::function<void(spConnection)> closecallback_;                    // 关闭fd_的回调函数，将回调TcpServer::closeconnection()
    std::function<void(spConnection)> errorcallback_;                    // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()
    std::function<void(spConnection, std::string &)> onmessagecallback_; // 处理报文的回调函数，将回调TcpServer::onmessage()
    std::function<void(spConnection)> sendcompletecallback_;             // 发送完数据后，通知TcpServer::sendcomplete()

public:
    Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock);
    ~Connection();

    int fd() const;         // 返回fd成员变量
    std::string ip() const; // 返回客户端的ip
    uint16_t port() const;  // 返回客户端的port

    void onmessage();     // 处理对端发送过来的消息，回调函数
    void closecallback(); // tcp连接断开的回调函数，供Channel回调
    void errorcallback(); // tcp连接出错的回调函数，供Channel回调
    void writecallback(); // 写事件的回调函数，供channel回调

    void setclosecallback(std::function<void(spConnection)> fn);                    // 设置关闭fd_的回调函数
    void seterrorcallback(std::function<void(spConnection)> fn);                    // 设置fd_发生了错误的回调函数
    void setonmessagecallback(std::function<void(spConnection, std::string &)> fn); // 设置处理报文的回调函数
    void setsendcompletecallback(std::function<void(spConnection)> fn);             // 发送数据完成后的回调函数

    void send(const char *data, size_t sz);             // 发送数据（任何线程都是调用此函数）
    void sendinloop(std::shared_ptr<std::string> data); // 发送数据的具体实现。如果是IO，直接使用，如果是工作线程，将此函数传递给IO线程

    bool timeout(time_t now, int val); // 判断TCP连接是否超时（空闲太久）
};

#endif