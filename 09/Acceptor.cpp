#include "Acceptor.h"

Acceptor::Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port):loop_(loop)
{
    // 创建非阻塞的监听socket
    servsock_ = new Socket(createnonblocking());

    // 设置一些提高性能的属性
    servsock_->setkeepalive(true);
    servsock_->setreuseaddr(true);
    servsock_->setreuseport(true);
    servsock_->settcpnodelay(true);

    // 服务端的地址和协议
    InetAddress servaddr(ip, port);
    servsock_->bind(servaddr); // 绑定ip和端口
    servsock_->listen();       // 开启监听

    acceptchannel_ = new Channel(loop_, servsock_->fd());
    acceptchannel_->setreadcallback(std::bind(&Channel::newconnection, acceptchannel_, servsock_));
    // 把ch加入epoll句柄中，监听读事件
    acceptchannel_->enablereading();
}

Acceptor::~Acceptor()
{
    delete servsock_;
    delete acceptchannel_;
}
