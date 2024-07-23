#include "TcpServer.h"

// 初始化eventloop
// 创建非阻塞的服务端监听socket，传递给epoll句柄，关联事件，开始监听
TcpServer::TcpServer(const std::string &ip, uint16_t port, int threadnum)
    : mainloop_(new EventLoop(true)), acceptor_(mainloop_.get(), ip, port), threadnum_(threadnum), threadpool_(threadnum_, "IO")
{
    // 主事件循环设置超时回调函数
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));

    // 设置回调函数
    acceptor_.setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));

    for (int i = 0; i < threadnum_; i++)
    {
        // 创建从事件循环
        subloops_.emplace_back(new EventLoop(false, 5, 10));
        subloops_[i]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
        subloops_[i]->settimercallback(std::bind(&TcpServer::removeconn, this, std::placeholders::_1));

        // 将从事件循环的事件循环放入IO线程池
        // 一条线程对应一个事件循环
        threadpool_.addtask(std::bind(&EventLoop::run, subloops_[i].get()));
    }
}

TcpServer::~TcpServer()
{
}

// 开启服务端循环监听
void TcpServer::start()
{
    mainloop_->run();
}

// 停止所有线程和事件循环
void TcpServer::stop()
{
    // 停止主从事件循环
    mainloop_->stop();
    logger.log(LogLevel::WARNING, "主事件循环已停止");
    for (int i = 0; i < threadnum_; i++)
    {
        subloops_[i]->stop();
    }
    logger.log(LogLevel::WARNING, "从事件循环已停止");
    // 停止IO线程
    threadpool_.stop();
    logger.log(LogLevel::WARNING, "IO线程池停止");
}

// 处理新的客户端连接请求
void TcpServer::newconnection(std::unique_ptr<Socket> clientsock)
{
    try
    {
        int tmpfd = clientsock->fd();
        logger.logFormatted(LogLevel::WARNING, "Before construct spConnection %d", tmpfd);
        // 创建新的Connection实例，并且给新建的conn绑定事件循环
        spConnection conn(new Connection(subloops_[clientsock->fd() % threadnum_].get(), std::move(clientsock)));
        logger.logFormatted(LogLevel::WARNING, "1After construct spConnection %d", tmpfd);
        
        // 设置断开/出错时的回调函数
        conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
        conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
        conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete, this, std::placeholders::_1));
        conn->setonmessagecallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));

        {
            std::lock_guard<std::mutex> gd(mutex_);
            conns_[conn->fd()] = conn; // 加入map容器
        }

        // 将新的conn加入事件循环的监听中
        subloops_[conn->fd() % threadnum_]->newconnection(conn);

        // 回调应用层代码EchoServer::HandleNewConnection()。
        if (newconnectioncb_)
            newconnectioncb_(conn);
    }
    catch (const std::exception &e)
    {
        logger.logFormatted(LogLevel::ERROR, "ADD new Connection Error: %s", e.what());
    }
}

// 关闭客户端的连接，在Connection类中回调此函数。
void TcpServer::closeconnection(spConnection conn)
{
    // 回调业务层实现
    if (closeconnectioncb_)
        closeconnectioncb_(conn);
    {
        std::lock_guard<std::mutex> gd(mutex_);
        // 关闭该客户端的fd。
        conns_.erase(conn->fd());
    }
}

// 客户端连接出错，回调并进行析构
void TcpServer::errorconnection(spConnection conn)
{
    if (errorconnectioncb_)
        errorconnectioncb_(conn); // 回调EchoServer::HandleError()。
    {
        std::lock_guard<std::mutex> gd(mutex_);
        // 关闭该客户端的fd。
        conns_.erase(conn->fd());
    }
}

// 处理客户端的请求报文，在Connection类中回调此函数
void TcpServer::onmessage(spConnection conn, std::string &message)
{
    // 回调EchoServer::HandleMessage()。
    if (onmessagecb_)
        onmessagecb_(conn, message);
}

// 数据发送完成后，在Connection类中回调此函数
void TcpServer::sendcomplete(spConnection conn)
{
    // 回调应用层实现
    if (sendcompletecb_)
        sendcompletecb_(conn);
}

// epoll_wait()超时，在EventLoop类中回调此函数
void TcpServer::epolltimeout(EventLoop *loop)
{
    // if (timeoutcb_)
    //     timeoutcb_(loop);
}

void TcpServer::setnewconnectioncb(std::function<void(spConnection)> fn)
{
    newconnectioncb_ = fn;
}

void TcpServer::setcloseconnectioncb(std::function<void(spConnection)> fn)
{
    closeconnectioncb_ = fn;
}

void TcpServer::seterrorconnectioncb(std::function<void(spConnection)> fn)
{
    errorconnectioncb_ = fn;
}

void TcpServer::setonmessagecb(std::function<void(spConnection, std::string &message)> fn)
{
    onmessagecb_ = fn;
}

void TcpServer::setsendcompletecb(std::function<void(spConnection)> fn)
{
    sendcompletecb_ = fn;
}

void TcpServer::settimeoutcb(std::function<void(EventLoop *)> fn)
{
    timeoutcb_ = fn;
}

// 删除空闲的客户端连接
void TcpServer::removeconn(int fd)
{
    {
        std::lock_guard<std::mutex> gd(mutex_);
        // 关闭该客户端的fd。
        conns_.erase(fd);
    }
}
