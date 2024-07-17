#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_
#include "Epoll.h"
#include <functional>
#include <memory>
#include <sys/syscall.h>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>

class Epoll;
class Channel;

// 事件循环
class EventLoop
{
private:
    std::unique_ptr<Epoll> ep_;                             // 每个事件循环只有一个epoll
    std::function<void(EventLoop *)> epolltimeoutcallback_; // epoll_wait()超时回调函数
    pid_t threadid_;                                        // 事件循环所在线程ID
    std::queue<std::function<void()>> taskqueue_;           // 事件循环线程被eventfd唤醒后执行的任务队列
    std::mutex mutex_;                                      // 任务队列同步的互斥锁
    int wakeupfd_;                                          // 声明一个eventfd
    std::unique_ptr<Channel> wakechannel_;                  // eventfd的channel
public:
    EventLoop();
    ~EventLoop();

    void run();                                                     // 运行事件循环
    void updatechannel(Channel *ch);                                // 把需要监视的ch对象添加到epoll句柄中
    void removechannel(Channel *ch);                                // 从红黑树上删除channel
    void setepolltimeoutcallback(std::function<void(EventLoop *)>); // 设置超时回调函数

    bool isinloopthread();                      // 判断当前线程是否为事件循环线程
    void queueinloop(std::function<void()> fn); // 入队的函数
    void wakeup();                              // 唤醒事件循环的函数
    void handlewakeup();                        // 事件循环线程被eventfd唤醒后，执行的函数
};

#endif