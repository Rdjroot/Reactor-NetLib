#include "TcpServer.h"

// 初始化eventloop
// 创建非阻塞的服务端监听socket
// 传递给epoll句柄，关联事件，开始监听
TcpServer::TcpServer(const std::string &ip, uint16_t port)
{
    acceptor_ = new Acceptor(&loop_, ip, port);
    // 设置回调函数
    acceptor_->setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1));
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
    // 当前打印有误，无法获得正确的ip和端口
    std::cout << "accept client(fd=" << conn->fd() << ", ip=" << conn->ip()
              << ", port=" << conn->port() << ") ok." << std::endl;
    // 设置断开/出错时的回调函数
    conn->setclosecallback(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
    conn->seterrorcallback(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));

    conns_[conn->fd()] = conn; // 加入容器
}

void TcpServer::closeconnection(Connection *conn)
{
    std::cout << "client(eventfd=" << conn->fd() << ") disconnected.\n";
    // 关闭该客户端的fd。
    conns_.erase(conn->fd());
    delete (conn);
}

void TcpServer::errorconnection(Connection *conn)
{
    std::cout << "client(eventfd=" << conn->fd() << ") error." << std::endl;
    conns_.erase(conn->fd());
    delete (conn);
}
