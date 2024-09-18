#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Connection.h"
#include <memory>

/**
 * Acceptor类：服务端监听类
 * （封装了服务端监听fd以及回调函数）
 * 
 * 用于监听新的连接，并返回给上层进行分发
 * [绑定了主事件循环，也是mainReactor核心部分]
*/
class Acceptor
{
private:
    EventLoop *loop_;                                              // Acceptor对应的事件循环，从构造函数传入，使用但不拥有（主事件循环）
    Socket servsock_;                                              // 服务端用于监听的socket，在构造函数中创建（拥有）
    Channel acceptchannel_;                                        // 对应的Channel，在构造函数中创建（拥有）
    std::function<void(std::unique_ptr<Socket>)> newconnectioncb_; // 处理客户端请求连接的回调函数
public:
    Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port);
    ~Acceptor();

    void newconnection(); // 处理客户端的新请求

    // 设置处理新客户端连接请求的回调函数，将在创建Acceptor对象的时候（TcpServer类的构造函数中）设置
    void setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn);
};

#endif