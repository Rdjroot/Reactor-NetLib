#include "TcpServer.h"

// 初始化eventloop
// 创建非阻塞的服务端监听socket
// 传递给epoll句柄，关联事件，开始监听
TcpServer::TcpServer(const std::string &ip, uint16_t port)
{
    acceptor_ = new Acceptor(&loop_, ip, port);
}

TcpServer::~TcpServer()
{
    delete acceptor_;
}

// 开启循环监听
void TcpServer::start()
{
    loop_.run();
}
