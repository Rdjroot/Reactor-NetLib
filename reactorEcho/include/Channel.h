#ifndef CHANNEL_H_
#define CHANNEL_H_
#include <sys/epoll.h>
#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "EventLoop.h"
#include <memory>

class EventLoop;

/**
 * Channel类：文件描述符（socket）的保姆，掌管处理事件的发生和变化(监听属性、类型等)
 * 
 * 作用：绑定回调函数、注册（取消）监听事件、记录发生的事件
 * [Channel和Epoll是多对一的关系，一个Channel只对应一个Epoll]
 * 
*/
class Channel
{
private:
    int fd_ = -1;     // Channel拥有的fd，Channel与fd 为一对一关系
    EventLoop *loop_; // loop_为上层传入，不属于Channel。

    bool inepoll_ = false; // 记录Channel是否已经添加到epoll句柄中
    uint32_t events_ = 0;  // fd_需要监视的事件，listenfd和clientfd需要监视EPOLLIN，clientfd可能需要监视EPOLLOUT
    uint32_t revents_ = 0; // 存储fd_已发生的事件

    std::function<void()> readcallback_;  // fd_读事件的回调函数，如果是acceptchannel，将回调Acceptor::newconnection()，如果是clientchannel，将回调Connection::onmessage()
    std::function<void()> closecallback_; // 关闭fd_的回调函数，将回调Connection::closecallback()
    std::function<void()> errorcallback_; // fd_发生了错误的回调函数，将回调Connection::errorcallback()
    std::function<void()> writecallback_; // fd_写事件的回调函数，将回调Connection::writecallback()

public:
    Channel(EventLoop *loop, int fd);
    ~Channel();

    // 获取成员数据
    int fd();
    bool inpoll();
    uint32_t events();
    uint32_t revents();

    void useet();                 // 采用边缘触发
    void enablereading();         // 让epoll_wait监视fd_的读事件，即注册读事件
    void disablereading();        // 取消读事件
    void enablewriting();         // 注册写事件
    void disablewriting();        // 取消写事件
    void disableall();            // 取消全部事件

    void remove();                // 从事件循环中删除Channel
    void setinepoll();            // 把inepoll的值设置为true
    void setrevents(uint32_t ev); // 设置revents_成员的值为ev
    
    void handleevent();             // 事件处理函数，epoll_wait()返回的时候，执行它

    void setreadcallback(std::function<void()> fn);  // 设置fd_读事件的回调函数
    void setclosecallback(std::function<void()> fn); // 设置关闭fd_的回调函数
    void seterrorcallback(std::function<void()> fn); // 设置fd_发生错误时的回调函数
    void setwritecallback(std::function<void()> fn); // 设置写事件的回调函数
};

#endif