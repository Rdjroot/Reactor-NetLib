#include "TcpServer.h"

// 初始化eventloop
// 创建非阻塞的服务端监听socket
// 传递给epoll句柄，关联事件，开始监听
TcpServer::TcpServer(const std::string &ip, uint16_t port,int threadnum):threadnum_(threadnum)
{
    mainloop_ = new EventLoop();        // 创建主事件循环 
    // 设置超时回调函数
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));

    // 绑定服务端监听的事件循环
    acceptor_ = new Acceptor(mainloop_, ip, port);
    // 设置回调函数
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));

    threadpool_ = new ThreadPool(threadnum_,"IO");       // 创建线程池

    for(int i = 0; i < threadnum_; i++)
    {
        // 创建从事件循环
        subloops_.push_back(new EventLoop);
        subloops_[i]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
        threadpool_->addtask(std::bind(&EventLoop::run,subloops_[i]));
    }
}

TcpServer::~TcpServer()
{
    delete mainloop_;
    delete acceptor_;
    // 释放所有Connection对象
    for (auto &aa : conns_)
    {
        delete aa.second;
    }

    for(auto &aa: subloops_)       // 释放从事件循环
        delete aa;

    delete threadpool_;         // 释放线程池
}

// 开启循环监听
void TcpServer::start()
{
    mainloop_->run();
}

// 处理 客户端新的连接请求
void TcpServer::newconnection(Socket *clientsock)
{

    // 把新建的conn分配给从事件循环
    Connection *conn = new Connection(subloops_[clientsock->fd()%threadnum_], clientsock);

    // 设置断开/出错时的回调函数
    conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
    conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete, this, std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&TcpServer::onmessage, this, std::placeholders::_1, std::placeholders::_2));

    conns_[conn->fd()] = conn; // 加入容器

    // 回调EchoServer::HandleNewConnection()。
    if (newconnectioncb_)
        newconnectioncb_(conn);
}

void TcpServer::closeconnection(Connection *conn)
{
    // 回调业务层实现
    if (closeconnectioncb_)
        closeconnectioncb_(conn);
    // 关闭该客户端的fd。
    conns_.erase(conn->fd());
    delete (conn);
}

void TcpServer::errorconnection(Connection *conn)
{
    if (errorconnectioncb_)
        errorconnectioncb_(conn); // 回调EchoServer::HandleError()。
    conns_.erase(conn->fd());
    delete (conn);
}

void TcpServer::onmessage(Connection *conn, std::string &message)
{
    if (onmessagecb_)
        onmessagecb_(conn, message);
    else
        std::cout << "no onmessagecb_" << std::endl;
}

void TcpServer::sendcomplete(Connection *conn)
{
    if (sendcompletecb_)
        sendcompletecb_(conn);
}

void TcpServer::epolltimeout(EventLoop *loop)
{
    if (timeoutcb_)
        timeoutcb_(loop);
}

void TcpServer::setnewconnectioncb(std::function<void(Connection *)> fn)
{
    newconnectioncb_ = fn;
}

void TcpServer::setcloseconnectioncb(std::function<void(Connection *)> fn)
{
    closeconnectioncb_ = fn;
}

void TcpServer::seterrorconnectioncb(std::function<void(Connection *)> fn)
{
    errorconnectioncb_ = fn;
}

void TcpServer::setonmessagecb(std::function<void(Connection *, std::string &message)> fn)
{
    onmessagecb_ = fn;
}

void TcpServer::setsendcompletecb(std::function<void(Connection *)> fn)
{
    sendcompletecb_ = fn;
}

void TcpServer::settimeoutcb(std::function<void(EventLoop *)> fn)
{
    timeoutcb_ = fn;
}
