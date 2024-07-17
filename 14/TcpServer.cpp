#include "TcpServer.h"

// 初始化eventloop
// 创建非阻塞的服务端监听socket
// 传递给epoll句柄，关联事件，开始监听
TcpServer::TcpServer(const std::string &ip, uint16_t port, int threadnum)
    : mainloop_(new EventLoop(true)), acceptor_(mainloop_.get(), ip, port), threadnum_(threadnum), threadpool_(threadnum_, "IO")
{
    // 设置超时回调函数
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));

    // 设置回调函数
    acceptor_.setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));

    for (int i = 0; i < threadnum_; i++)
    {
        // 创建从事件循环
        subloops_.emplace_back(new EventLoop(false,5,10));
        subloops_[i]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
        subloops_[i]->settimercallback(std::bind(&TcpServer::removeconn, this, std::placeholders::_1));
        threadpool_.addtask(std::bind(&EventLoop::run, subloops_[i].get()));
    }
}

TcpServer::~TcpServer()
{
}

// 开启循环监听
void TcpServer::start()
{
    mainloop_->run();
}

// 处理 客户端新的连接请求
void TcpServer::newconnection(std::unique_ptr<Socket> clientsock)
{
    // 把新建的conn分配给从事件循环
    spConnection conn(new Connection(subloops_[clientsock->fd() % threadnum_].get(), std::move(clientsock)));

    // 设置断开/出错时的回调函数
    conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
    conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete, this, std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));
    {
        std::lock_guard<std::mutex> gd(mutex_);
        conns_[conn->fd()] = conn; // 加入容器
    }
    subloops_[conn->fd() % threadnum_]->newconnection(conn);

    // 回调EchoServer::HandleNewConnection()。
    if (newconnectioncb_)
        newconnectioncb_(conn);
}

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

void TcpServer::onmessage(spConnection conn, std::string &message)
{
    if (onmessagecb_)
        onmessagecb_(conn, message);
    else
        std::cout << "no onmessagecb_" << std::endl;
}

void TcpServer::sendcomplete(spConnection conn)
{
    if (sendcompletecb_)
        sendcompletecb_(conn);
}

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

void TcpServer::removeconn(int fd)
{
    {
        std::lock_guard<std::mutex> gd(mutex_);
        // 关闭该客户端的fd。
        conns_.erase(fd);
    }
}
