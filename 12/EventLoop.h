#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_
#include "Epoll.h"
#include <functional>
#include<memory>

class Epoll;
class Channel;

// 事件循环
class EventLoop
{
private:
    std::unique_ptr<Epoll> ep_;                                             // 每个事件循环只有一个epoll
    std::function<void(EventLoop *)> epolltimeoutcallback_; // epoll_wait()超时回调函数
public:
    EventLoop();
    ~EventLoop();

    void run();                                                     // 运行事件循环
    void updatechannel(Channel *ch);                                // 把需要监视的ch对象添加到epoll句柄中
    void removechannel(Channel *ch);                                // 从红黑树上删除channel
    void setepolltimeoutcallback(std::function<void(EventLoop *)>); // 设置超时回调函数
};

#endif