#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_
#include "Epoll.h"

class Epoll;
class Channel;

class EventLoop
{
private:
    Epoll *ep_;            // 每个事件循环只有一个epoll

public:
    EventLoop();
    ~EventLoop();

    void run();             // 运行事件循环
    void updatechannel(Channel *ch);            // 把需要监视的ch对象添加到epoll句柄中
};

#endif