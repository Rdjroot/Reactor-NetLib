#include "Epoll.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

Epoll::Epoll()
{
    if ((epollfd_ = epoll_create(1)) == -1) // 创建epoll句柄（红黑树）。
    {
        logger.logFormatted(LogLevel::ERROR, "epoll_create() failed(%d).", errno);
        exit(-1);
    }
}

Epoll::~Epoll()
{
    close(epollfd_);
}

// 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
void Epoll::updatechannel(Channel *ch)
{
    // 为fd准备事件。
    epoll_event ev;           // 声明事件的数据结构。
    ev.data.ptr = ch;         // 指定Channel
    ev.events = ch->events(); // 指定事件(读事件、写事件;水平触发|边缘触发)

    // 如果已经在红黑树中
    if (ch->inpoll())
    {
        // 更新监听事件模式
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1)
        {
            logger.logFormatted(LogLevel::ERROR, "epoll_ctl() for MOD failed(%d).", errno);
            exit(-1);
        }
    }
    else
    {
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1)
        {
            logger.logFormatted(LogLevel::ERROR, "epoll_ctl() for ADD failed(%d).", errno);
            exit(-1);
        }
        ch->setinepoll(); // 标记已经入树
    }
}

// 从红黑树上删除channeL
void Epoll::removechannel(Channel *ch)
{
    // 如果已经在红黑树中
    if (ch->inpoll())
    {
        if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, ch->fd(), 0) == -1)
        {
            logger.logFormatted(LogLevel::ERROR, "epoll_ctl() for DEL failed(%d).", errno);
            exit(-1);
        }
    }
}

// 运行epoll_wait(), 等待事件发生，已发生的事件用vector容器返回
std::vector<Channel *> Epoll::loop(int timeout)
{
    std::vector<Channel *> channels = {};

    bzero(events_, sizeof(events_));

    // 等待监视的fd有事件发生。
    // timeout=-1 表示需要无限等待，无超时
    int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);

    // 返回失败。
    if (infds < 0)
    {
        // EBADF ：epfd不是一个有效的描述符。
        // EFAULT ：参数events指向的内存区域不可写。
        // EINVAL ：epfd不是一个epoll文件描述符，或者参数maxevents小于等于0。
        // EINTR ：阻塞过程中被信号中断，epoll_pwait()可以避免，或者错误处理中，解析error后重新调用epoll_wait()。
        // 在Reactor模型中，不建议使用信号，因为信号处理起来很麻烦，没有必要。------ 陈硕
        logger.log(LogLevel::ERROR, "epoll_wait() failed.");
        exit(-1);
    }

    // 超时。
    if (infds == 0)
    {
        // epoll超時了，代表系统很空闲，返回的channel为空
        // cerr << "epoll_wait() timeout." << endl;
        return channels;
    }

    // 如果infds>0，表示有事件发生的fd的数量
    for (int ii = 0; ii < infds; ii++)
    {
        // 取出一开始传入的ch，这个ch对应着一个socket
        // 这里能取出的ch，也是有事件发生的socket
        Channel *ch = (Channel *)events_[ii].data.ptr;
        ch->setrevents(events_[ii].events); // 设置channel的发生事件成员
        channels.push_back(ch);
    }

    return channels;
}