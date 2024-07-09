#include "Connection.h"

Connection::Connection(EventLoop *loop, Socket *clientsock) : loop_(loop), clientsock_(clientsock)
{
    // 为新客户端连接准备读事件和属性设置，并添加到epoll中。
    clientchannel_ = new Channel(loop_, clientsock_->fd());
    // 绑定回调函数
    clientchannel_->setreadcallback(std::bind(&Channel::onmessage, clientchannel_));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));
    clientchannel_->useet();         // 设置边缘触发，
    clientchannel_->enablereading(); // 将新的客户端fd的读事件添加到epoll中
}

Connection::~Connection()
{
    delete clientsock_;
    delete clientchannel_;
}

int Connection::fd() const
{
    return clientsock_->fd();
}

std::string Connection::ip() const
{
    return clientsock_->ip();
}

uint16_t Connection::port() const
{
    return clientsock_->port();
}

//
void Connection::closecallback()
{
    closecallback_(this);
}

void Connection::errorcallback()
{
    errorcallback_(this);
}

void Connection::setclosecallback(std::function<void(Connection *)> fn)
{
    closecallback_ = fn;
}

void Connection::seterrorcallback(std::function<void(Connection *)> fn)
{
    errorcallback_ = fn;
}
