#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include<functional>
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

class Acceptor
{
private:
    /* data */
    EventLoop *loop_;        // Acceptor对应的事件循环，从构造函数传入，使用但不拥有
    Socket *servsock_;       // 服务端用于监听的socket
    Channel *acceptchannel_;    // 对应的Channel，在构造函数中创建
public:
    Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port);
    ~Acceptor();
};



#endif