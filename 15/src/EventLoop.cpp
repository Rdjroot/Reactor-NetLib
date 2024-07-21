#include "EventLoop.h"

// 创建一个定时器文件描述符
int createtimefd(int sec = 30)
{
    // 创建timerfd
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);

    struct itimerspec timeout = {}; // 定时时间的数据结构
    timeout.it_value.tv_sec = sec;  // 设置定时时间
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd, 0, &timeout, 0);
    return tfd;
}

// 构造函数
EventLoop::EventLoop(bool mainloop, int timeval, int timeout)
            : ep_(new Epoll), wakeupfd_(eventfd(0, EFD_NONBLOCK))
            , wakechannel_(new Channel(this, wakeupfd_)), timefd_(createtimefd(timeout))
            , timerchannel_(new Channel(this, timefd_)), mainloop_(mainloop)
            , timeval_(timeval), timeout_(timeout), stop_(false)
{
    // 设置读事件
    wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup, this));
    wakechannel_->enablereading(); // 注册读事件，只要事件循环被环迅，就會調用handlewakeup

    timerchannel_->setreadcallback(std::bind(&EventLoop::handletimer, this));
    timerchannel_->enablereading();
}

EventLoop::~EventLoop()
{
}

// 运行事件循环
void EventLoop::run()
{
    threadid_ = syscall(SYS_gettid); // 获取事件循环所在线程的id
    while (!stop_)
    {
        // 不停地使用epoll_wait()监听事件
        // 发生的事件存放在channels中
        std::vector<Channel *> channels = ep_->loop(10 * 1000);

        if (channels.empty())
        {
            // 超时
            epolltimeoutcallback_(this);
        }

        for (auto &ch : channels)
        {
            // 处理事件
            ch->handleevent();
        }
    }
}

// 停止事件循环
void EventLoop::stop()
{
    stop_ = true;
    wakeup(); // 唤醒事件循环，如果没有这行代码，事件循环会在下次闹钟响或者epoll_wait()超时时才会停下来
}

// 更新监听事件类型
void EventLoop::updatechannel(Channel *ch)
{
    ep_->updatechannel(ch);
}

// 设置的是TcpServer::epolltimeout
void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop *)> fn)
{
    epolltimeoutcallback_ = fn;
}

// 从epoll句柄中删除该socket
void EventLoop::removechannel(Channel *ch)
{
    ep_->removechannel(ch);
}

// 判断当前线程是否为事件循环线程
bool EventLoop::isinloopthread()
{
    // 比较之前获得线程id和当前ID
    return threadid_ == syscall(SYS_gettid);
}

// 把任务添加到队列中
void EventLoop::queueinloop(std::function<void()> fn)
{
    {
        std::lock_guard<std::mutex> gd(mutex_);
        taskqueue_.push(fn);
    }
    // 唤醒事件循环
    wakeup();
}

// 用eventfd唤醒事件循环线程
void EventLoop::wakeup()
{
    // 作用是唤醒线程，不在乎写入数据是什么
    uint64_t val = 1;
    ssize_t result = write(wakeupfd_, &val, sizeof(val));
    if (result < 0)
    {
        logger.log(LogLevel::ERROR, "Failed to write to wakeupfd.");
    }
}

// 事件循环线程被eventfd唤醒后，执行的函数
void EventLoop::handlewakeup()
{
    uint64_t val;
    ssize_t result = read(wakeupfd_, &val, sizeof(val)); // 从eventfd中读取数据，如果不读取，eventfd读事件会一直触发

    if (result < 0)
    {
        logger.log(LogLevel::ERROR, "Failed to read from wakeupfd.");
    }

    std::function<void()> fn;
    std::lock_guard<std::mutex> gd(mutex_); // 给任务队列加锁

    // 执行队列中全部的发送任务
    while (!taskqueue_.empty())
    {
        fn = std::move(taskqueue_.front());
        taskqueue_.pop();
        fn(); // 执行任务,发送数据
    }
}

// 闹钟响时执行的函数
void EventLoop::handletimer()
{
    struct itimerspec timeout = {};     // 定时时间的数据结构
    timeout.it_value.tv_sec = timeval_; // 设置定时时间
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timefd_, 0, &timeout, 0);       // 重新设置定时时间

    if (mainloop_)
    {
        // std::cout << "主事件循环的闹钟响了" << std::endl;
    }
    else
    {
        // std::cout << "从事件循环的闹钟时间到了" << std::endl;
        time_t now = time(0); // 获取当前时间
        for (auto it = conns_.begin(); it != conns_.end();)
        {
            if (it->second->timeout(now, timeout_))
            {
                timercallback_(it->first);              // 从TcpServer的map中删除超时的conn。
                std::lock_guard<std::mutex> gd(mmutex_);
                it = conns_.erase(it);                  // 从EventLoop的map中删除超时的conn。
            }
            else
                it++;
        }
    }
}

// 把Connection对象保存在conns_中
void EventLoop::newconnection(spConnection conn)
{
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_[conn->fd()] = conn;
}

// 将被设置为TcpServer::removeconn()
void EventLoop::settimercallback(std::function<void(int)> fn)
{
    timercallback_ = fn;
}