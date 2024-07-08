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

std::vector<epoll_event> Epoll::loop(int timeout)
{
    // 存放epoll_wait()返回的事件
    std::vector<epoll_event> evs = {};

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
        return evs;
    }

    for(int ii = 0; ii < infds; ii++)
    {
        evs.push_back(events_[ii]);
    }

    return evs;
}
