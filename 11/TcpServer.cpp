#include "TcpServer.h"

// 初始化eventloop
// 创建非阻塞的服务端监听socket
// 传递给epoll句柄，关联事件，开始监听
TcpServer::TcpServer(const std::string &ip, uint16_t port)
{
    acceptor_ = new Acceptor(&loop_, ip, port);
    // 设置回调函数
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));
    loop_.setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
    delete acceptor_;
    for (auto &aa : conns_)
    {
        delete aa.second;
    }
}

// 开启循环监听
void TcpServer::start()
{
    loop_.run();
}

// 处理 客户端新的连接请求
void TcpServer::newconnection(Socket *clientsock)
{
    // std::cout << "222222accept client(fd=" << clientsock->fd() << ", ip=" << clientsock->ip()
    //             << ", port=" << clientsock->port() << ") ok." << std::endl;
    // 该对象没有被释放
    Connection *conn = new Connection(&loop_, clientsock);

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

void TcpServer::onmessage(Connection *conn, std::string message)
{
    if (onmessagecb_)
        onmessagecb_(conn, message);
    else
        std::cout<<"no onmessagecb_"<<std::endl;
}

void TcpServer::sendcomplete(Connection *conn)
{
    if(sendcompletecb_)
        sendcompletecb_(conn);
}

void TcpServer::epolltimeout(EventLoop *loop)
{
    if(timeoutcb_)
        timeoutcb_(loop);
}

void TcpServer::setnewconnectioncb(std::function<void(Connection *)> fn)
{
    std::cout << "setnewconnectioncb ok" <<std::endl;
    newconnectioncb_ = fn;
}

void TcpServer::setcloseconnectioncb(std::function<void(Connection *)> fn)
{
    std::cout << "setcloseconnectioncb ok" <<std::endl;
    closeconnectioncb_ = fn;
}

void TcpServer::seterrorconnectioncb(std::function<void(Connection *)> fn)
{
    std::cout << "seterrorconnectioncb ok" <<std::endl;
    errorconnectioncb_ = fn;
}

void TcpServer::setonmessagecb(std::function<void(Connection *, std::string &message)> fn)
{
    std::cout << "setonmessagecb ok" <<std::endl;
    onmessagecb_ = fn;
}

void TcpServer::setsendcompletecb(std::function<void(Connection *)> fn)
{
    std::cout << "setsendcompletecb ok" <<std::endl;
    sendcompletecb_ = fn;
}

void TcpServer::settimeoutcb(std::function<void(EventLoop *)> fn)
{
    std::cout << "settimeoutcb ok" <<std::endl;
    timeoutcb_ = fn;
}
