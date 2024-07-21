#include "Channel.h"

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd)
{
}

Channel::~Channel()
{
    // 对于fd_和ep_，本类都只是使用它，它们不属于本类，所以不能销毁和关闭
}

// 返回fd_成员
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

// 设置监视读事件
void Channel::enablereading()
{
    events_ |= EPOLLIN;
    loop_->updatechannel(this);
}

// 取消监听读事件
void Channel::disablereading()
{
    events_ &= ~EPOLLIN;
    loop_->updatechannel(this);
}

// 设置监视写事件
void Channel::enablewriting()
{
    events_ |= EPOLLOUT;
    loop_->updatechannel(this);
}

// 取消监听写事件
void Channel::disablewriting()
{
    events_ &= ~EPOLLOUT;
    loop_->updatechannel(this);
}

// 取消所有监听事件
void Channel::disableall()
{
    events_ = 0;
    loop_->updatechannel(this);
}

// 从事件循环中删除Channel
void Channel::remove()
{
    disableall();               // 先取消全部的事件
    loop_->removechannel(this); // 从epoll句柄红黑树上删除fd
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

// 处理发生的事件
void Channel::handleevent()
{
    // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    // EPOLLRDHUP 表示对端关闭连接或半关闭。
    if (revents_ & EPOLLRDHUP)
    {
        closecallback_();       // 回调Connection::closecallback()
    }
    //  EPOLLIN 普通数据  EPOLLPRI带外数据
    else if (revents_ & (EPOLLIN | EPOLLPRI))   // 接收缓冲区中有数据可以读。
    {
        // 如果是新的连接，这里fd绑定的应该是Acceptor::newconnection()
        // 如果fd是客户端，这里应该绑定的应该是Connection::onmessage()
        readcallback_();
    }
    else if (revents_ & EPOLLOUT) // 有数据需要写，
    {
        writecallback_();   // 回调Connection::writecallback()
    }
    else // 其它事件，都视为错误。
    {
        errorcallback_();   // 回调Connection::errorcallback()
    } 
}

// 设置fd_读事件的回调函数
void Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_ = fn;
}

// 设置关闭fd_的回调函数
void Channel::setclosecallback(std::function<void()> fn)
{
    closecallback_ = fn;
}

// 设置fd_发生了错误的回调函数
void Channel::seterrorcallback(std::function<void()> fn)
{
    errorcallback_ = fn;
}

// 设置写事件的回调函数
void Channel::setwritecallback(std::function<void()> fn)
{
    writecallback_ = fn;
}
