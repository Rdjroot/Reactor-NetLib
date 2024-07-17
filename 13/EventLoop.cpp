#include "EventLoop.h"
#include "EventLoop.h"

EventLoop::EventLoop()
    :ep_(new Epoll),wakeupfd_(eventfd(0,EFD_NONBLOCK)),wakechannel_(new Channel(this,wakeupfd_))
{
    // 设置读事件
    wakechannel_->setreadcallback(std::bind(&EventLoop::handlewakeup,this));
    wakechannel_->enablereading();          // 注册读事件，只要事件循環被喚醒，就會調用handlewakeup
}

EventLoop::~EventLoop()
{
}

void EventLoop::run()
{
    threadid_ = syscall(SYS_gettid);        // 获取事件循环所在线程的id
    while(true)
    {
        // 不停地使用epoll_wait()监听事件
        // 然后存放到channels中
        std::vector<Channel *> channels = ep_->loop(10*1000);

        if(channels.empty())
        {
            epolltimeoutcallback_(this);
        }

        for(auto &ch: channels)
        {
            // 处理事件
            ch->handleevent();
        }
    }
}

void EventLoop::updatechannel(Channel *ch)
{
    ep_->updatechannel(ch);
}

void EventLoop::setepolltimeoutcallback(std::function<void(EventLoop *)> fn)
{
    epolltimeoutcallback_ = fn;
}

void EventLoop::removechannel(Channel *ch)
{
    ep_->removechannel(ch);
}

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

void EventLoop::wakeup()
{
    // 作用是唤醒线程，不在乎写入数据是什么
    uint64_t val = 1;
    write(wakeupfd_,&val,sizeof(val));
}

// 事件循环线程被eventfd唤醒后，执行的函数
void EventLoop::handlewakeup()
{
    std::cout<<"handlewakeup() thread id is " << syscall(SYS_gettid) <<".\n";
    uint64_t val;
    read(wakeupfd_,&val,sizeof(val));   // 从eventfd中读取数据，如果不读取，eventfd读事件会一直触发

    std::function<void()> fn;
    std::lock_guard<std::mutex> gd(mutex_);     //给任务队列加锁

    // 发送数据
    while(!taskqueue_.empty())
    {
        fn = std::move(taskqueue_.front());
        taskqueue_.pop();
        fn();       // 执行任务
    }
}
