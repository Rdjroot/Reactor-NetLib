#include "Channel.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd)
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
    events_ |= EPOLLIN;
    loop_->updatechannel(this);
}

void Channel::disablereading()
{
    events_ &= ~EPOLLIN;
    loop_->updatechannel(this);
}

void Channel::enablewriting()
{
    events_ |= EPOLLOUT;
    loop_->updatechannel(this);
}

void Channel::disablewriting()
{
    events_ &= ~EPOLLOUT;
    loop_->updatechannel(this);
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
        closecallback_();
    }
    //  EPOLLIN 普通数据  EPOLLPRI带外数据
    else if (revents_ & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
    {
        // 如果是新的连接，这里fd绑定的应该是newconnection
        // 如果fd是客户端，这里应该绑定的应该是 onmessage
        readcallback_();
    }
    else if (revents_ & EPOLLOUT) // 有数据需要写，
    {
        writecallback_();
    }
    else // 其它事件，都视为错误。
    {
        errorcallback_();
    }
}

void Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_ = fn;
}

void Channel::setclosecallback(std::function<void()> fn)
{
    closecallback_ = fn;
}

void Channel::seterrorcallback(std::function<void()> fn)
{
    errorcallback_ = fn;
}


void Channel::setwritecallback(std::function<void()> fn)
{
    writecallback_ = fn;
}
