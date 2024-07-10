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

class Channel;
class EventLoop;

// 客户端Channel
class Connection
{
private:
    /* data */
    EventLoop *loop_;        // Connection对应的事件循环，从构造函数传入，使用但不拥有
    Socket *clientsock_;     // 与客户端通信的socket
    Channel *clientchannel_; // 对应的Channel，在构造函数中创建
    Buffer inputbuffer_;     // 接收缓冲区
    Buffer outputbuffer_;    // 发送缓冲区

    std::function<void(Connection *)> closecallback_;
    std::function<void(Connection *)> errorcallback_;

public:
    Connection(EventLoop *loop, Socket *clientsock);
    ~Connection();

    int fd() const; // 返回fd成员变量
    std::string ip() const;
    uint16_t port() const;

    void onmessage();                               // 处理对端发送过来的消息

    void closecallback();                                        // tcp连接断开的回调函数
    void errorcallback();                                        // tcp连接出错的回调函数
    void setclosecallback(std::function<void(Connection *)> fn); // tcp连接断开的回调函数
    void seterrorcallback(std::function<void(Connection *)> fn); // tcp连接出错的回调函数
};

#endif