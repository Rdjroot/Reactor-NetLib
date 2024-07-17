#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Buffer.h"
#include "unistd.h"
#include "sys/socket.h"
#include<atomic>
#include<memory>

class Connection;
using spConnection = std::shared_ptr<Connection>;


// 客户端Channel
class Connection:public std::enable_shared_from_this<Connection>
{
private:
    /* data */
    const std::unique_ptr<EventLoop>& loop_;        // Connection对应的事件循环，从构造函数传入，使用但不拥有
    std::unique_ptr<Socket> clientsock_;     // 与客户端通信的socket(外界传入，但管理其生命周期)
    std::unique_ptr<Channel> clientchannel_; // 对应的Channel，在构造函数中创建（拥有）
    Buffer inputbuffer_;     // 接收缓冲区
    Buffer outputbuffer_;    // 发送缓冲区
    std::atomic_bool disconnect_;       // 客户端是否已断开，如果已断开，设为true

    std::function<void(spConnection)> closecallback_;
    std::function<void(spConnection)> errorcallback_;
    std::function<void(spConnection, std::string&)> onmessagecallback_;
    std::function<void(spConnection)> sendcompletecallback_;        // 发送完数据后，通知tcpserver

public:
    Connection(const std::unique_ptr<EventLoop>& loop, std::unique_ptr<Socket>clientsock);
    ~Connection();

    int fd() const; // 返回fd成员变量
    std::string ip() const;
    uint16_t port() const;

    void onmessage();     // 处理对端发送过来的消息，回调函数
    void closecallback(); // tcp连接断开的回调函数
    void errorcallback(); // tcp连接出错的回调函数
    void writecallback();       // 写事件的回调函数，供channel回调

    void setclosecallback(std::function<void(spConnection)> fn);                  // 设置tcp连接断开的回调函数
    void seterrorcallback(std::function<void(spConnection)> fn);                  // 设置tcp连接出错的回调函数
    void setonmessagecallback(std::function<void(spConnection, std::string&)> fn); // 设置处理报文的回调函数
    void setsendcompletecallback(std::function<void(spConnection)> fn);

    void send(const char *data, size_t sz);
};

#endif