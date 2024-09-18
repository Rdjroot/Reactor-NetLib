#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include <functional>
#include <memory>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h> // 定时器需要包含这个头文件
#include <unordered_map>
#include <atomic>
#include "Connection.h"
#include "Logger.h"
#include "Epoll.h"

class Epoll;
class Channel;
class Connection;
using spConnection = std::shared_ptr<Connection>;

extern Logger &logger;

/**
 * EventLoop: 事件循环类，持续循环的获取监听结果并根据结果调用处理函数
 *      （整合封装了Epoll和Channel，向上提供更简便的接口）
 *
 * 功能：管理fd事件（增删改）、开启（关闭）事件循环、定时器清理空闲fd、
 *      工作线程和IO线程交互结点（提供wakeupfd_给工作线程唤醒IO线程）
 * 
 * [EventLoop与Thread一对一绑定，EventLoop与EPoll也是一对一]
*/
class EventLoop
{
private:
    std::unique_ptr<Epoll> ep_;             // 每个事件循环只有一个epoll红黑树
    int wakeupfd_;                          // 声明一个用于唤醒的eventfd
    std::unique_ptr<Channel> wakechannel_;  // eventfd的channel
    int timefd_;                            // 定时器的fd
    std::unique_ptr<Channel> timerchannel_; // 定时器的channel

    bool mainloop_;         // 代表当前是什么类别的事件循环 true- 主事件循环，false - 从事件循环
    int timeval_;           // 闹钟时间间隔，单位：秒
    int timeout_;           // Connection对象超时的时间，单位：秒
    std::atomic_bool stop_; // 事件循环标志位，是否运行中

    pid_t threadid_;                              // 事件循环所在线程ID
    std::queue<std::function<void()>> taskqueue_; // 事件循环线程被eventfd唤醒后执行的任务队列
    std::mutex mutex_;                            // 任务队列同步的互斥锁

    std::unordered_map<int, spConnection> conns_; // map容器，存放connection对象，{fd: conn}，代表当前放在epoll红黑树中的conn
    std::mutex mmutex_;                           // 保护conns_的互斥锁

    std::function<void(int)> timercallback_;                // 定时器超时回调
    std::function<void(EventLoop *)> epolltimeoutcallback_; // epoll_wait()超时回调

public:
    EventLoop(bool mainloop, int timeval = 30, int timeout = 80);
    ~EventLoop();

    void run();  // 运行事件循环
    void stop(); // 停止事件循环

    void updatechannel(Channel *ch);            // 把需要监视的ch对象添加到epoll句柄中
    void removechannel(Channel *ch);            // 从红黑树上删除channel
    void newconnection(spConnection conn);      // 往map容器中添加connection对象
    void queueinloop(std::function<void()> fn); // 入队的函数

    bool isinloopthread();  // 判断当前线程是否为事件循环线程

    void wakeup();          // 唤醒事件循环的函数
    void handlewakeup();    // 事件循环线程被eventfd唤醒后，执行的函数
    void handletimer();     // 定时器时间一到，执行的函数

    void settimercallback(std::function<void(int)>);                // 设置定时器超时回调
    void setepolltimeoutcallback(std::function<void(EventLoop *)>); // 设置超时回调函数
};

#endif