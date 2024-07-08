#include "Channel.h"

Channel::Channel(Epoll *ep, int fd):ep_(ep),fd_(fd)
{

}

Channel::~Channel()
{
    // 对于fd_和ep_，本类都只是使用它，它们不属于本类，所以不能销毁和关闭
}

int Channel::fd()
{
    return fd_;
}
// 判断Channel是否在红黑树上
bool Channel::inpoll()
{
    return inepoll_;
}

// 需要监视的事件
uint32_t Channel::events()
{
    return events_;
}

// 已发生的事件
uint32_t Channel::revents()
{
    return revents_;
}

// 采用边缘触发
void Channel::useet()
{
    events_ = events_ | EPOLLET;
}

// 监视读事件
void Channel::enablereading()
{
    ep_->updatechannel(this);
}

// 记录Channel已添加到epoll的红黑树中
void Channel::setinepoll()
{
    inepoll_ = true;
}

// 设置已发生的事件
void Channel::setrevents(uint32_t ev)
{
    revents_ = ev;
}

void Channel::handleevent()
{
}

void Channel::newconnection(Socket *servsock)
{
}

void Channel::onmessage()
{
}

void Channel::setreadcallback(std::function<void()> fn)
{
    
}
