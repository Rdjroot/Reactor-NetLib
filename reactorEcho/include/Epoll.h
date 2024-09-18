#ifndef EPOLL_H_
#define EPOLL_H_
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <sys/epoll.h>
#include <vector>
#include <unistd.h>
#include "Channel.h"
#include <cstring>
#include "Logger.h"

extern Logger &logger;

class Channel;

/**
 * Epoll: 封装epoll的数据结构，掌管epoll句柄的增删和红黑树的事件监听
 * 
 * 作用：监听事件是否触发，返回发生事件的fd，返回具体事件
 * [与Reactor/EpollEvent是一对一的关系，与Channel是一对多]
*/
class Epoll
{
private:
    static const int MaxEvents = 100; // 默认的返回大小
    int epollfd_ = -1;                // 初始化epoll句柄，在构造函数中创建
    epoll_event events_[MaxEvents];   // 存放epoll_wait返回事件的数组
public:
    Epoll();
    ~Epoll();

    void updatechannel(Channel *ch);                      // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void removechannel(Channel *ch);                      // 从红黑树上删除channel
    std::vector<Channel *> loop(int timeout = 100000000); // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回
};

#endif