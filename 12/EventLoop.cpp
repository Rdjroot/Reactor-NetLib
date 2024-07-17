#include "EventLoop.h"
#include "EventLoop.h"

EventLoop::EventLoop():ep_(new Epoll)
{
}

EventLoop::~EventLoop()
{
}

void EventLoop::run()
{
    while(true)
    {
        // 不停地使用epoll_wait()监听事件
        // 然后存放到channels中
        std::vector<Channel *> channels = ep_->loop();

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