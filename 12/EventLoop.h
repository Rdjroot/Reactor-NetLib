#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_
#include "Epoll.h"
#include <functional>

class Epoll;
class Channel;

// 事件循环
class EventLoop
{
private:
    Epoll *ep_;                                             // 每个事件循环只有一个epoll
    std::function<void(EventLoop *)> epolltimeoutcallback_; // epoll_wait()超时回调函数
public:
    EventLoop();
    ~EventLoop();

    void run();                      // 运行事件循环
    void updatechannel(Channel *ch); // 把需要监视的ch对象添加到epoll句柄中
    void setepolltimeoutcallback(std::function<void(EventLoop *)>);
};

#endif