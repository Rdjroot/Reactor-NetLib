#include "Epoll.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

Epoll::Epoll()
{
    if ((epollfd_ = epoll_create(1)) == -1) // 创建epoll句柄（红黑树）。
    {
        cerr << "epoll_create() failed(" << errno << ")." << endl;
        exit(-1);
    }
}

Epoll::~Epoll()
{
    close(epollfd_);
}

/**
 * 给epoll句柄添加需要监视的socket
 * op表示监听的事件（读事件、写事件）
 */
void Epoll::addfd(int fd, uint32_t op)
{
    // 为fd准备事件。
    epoll_event ev;  // 声明事件的数据结构。
    ev.data.fd = fd; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events = op;  // 采用水平触发。

    // 把需要监视的listenfd和它的事件加入epollfd中。
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        cout << "epoll_ctl() failed(" << errno << ")." << endl;
        exit(-1);
    }
}

// 更新Chanenl，将需要监听的fd和Channel绑定
void Epoll::updatechannel(Channel *ch)
{
    // 为fd准备事件。
    epoll_event ev;           // 声明事件的数据结构。
    ev.data.ptr = ch;         // 指定Channel
    ev.events = ch->events(); // 指定事件

    // 如果已经在红黑树中
    if (ch->inpoll())
    {
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1)
        {
            cout << "epoll_ctl() failed(" << errno << ")." << endl;
            exit(-1);
        }
    }
    else
    {
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1)
        {
            cout << "epoll_ctl() failed(" << errno << ")." << endl;
            exit(-1);
        }
        ch->setinepoll();           // 标记已经入树
    }
}

// 运行epoll_wait(), 等待事件发生，已发生的事件用vector容器返回
std::vector<Channel *> Epoll::loop(int timeout)
{
    std::vector<Channel *>channels = {};

    // 等待监视的fd有事件发生。
    // -1 表示需要无限等待
    int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);
    
    // 返回失败。
    if (infds < 0)
    {
        cerr << "epoll_wait() failed" << endl;
        exit(-1);
    }

    // 超时。
    if (infds == 0)
    {
        cerr << "epoll_wait() timeout." << endl;
        return channels;
    }

    

    for(int ii = 0; ii < infds; ii++)
    {
        // 取出一开始传入的ch，这个ch对应着一个socket
        // 这里能取出的ch，也是有事件发生的socket
        Channel *ch = (Channel *)events_[ii].data.ptr;
        ch->setrevents(events_[ii].events);         // 设置channel的发生事件成员
        channels.push_back(ch);
    }

    return channels;
}