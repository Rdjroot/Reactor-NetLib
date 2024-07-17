#ifndef EPOLL_H_
#define EPOLL_H_
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <sys/epoll.h>
#include <vector>
#include <unistd.h>
#include <iostream>
#include "Channel.h"
#include <cstring>

class Channel;

// 封装Epoll的数据结构，掌管epoll句柄的增删和 红黑树的事件监听
class Epoll
{
private:
    static const int MaxEvents = 100; // 默认的返回大小
    int epollfd_ = -1;                // 初始化epoll句柄，在构造函数中创建
    epoll_event events_[MaxEvents];   // 存放epoll_wait返回事件的数组
public:
    Epoll();
    ~Epoll();

    // void addfd(int fd, uint32_t op);                 // 把需要监视的fd添加到epoll句柄（红黑树）上
    // std::vector<epoll_event> loop(int timeout = -1); // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。

    void updatechannel(Channel *ch);                // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void removechannel(Channel *ch);
    std::vector<Channel *> loop(int timeout = 100000000);  // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回
};

# endif