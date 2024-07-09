#include "TcpServer.h"

// 初始化eventloop
// 创建非阻塞的服务端监听socket
// 传递给epoll句柄，关联事件，开始监听
TcpServer::TcpServer(const std::string &ip, uint16_t port)
{
    // 创建非阻塞的监听socket
    Socket *servsock = new Socket(createnonblocking());

    // 设置一些提高性能的属性
    servsock->setkeepalive(true);
    servsock->setreuseaddr(true);
    servsock->setreuseport(true);
    servsock->settcpnodelay(true);

    // 服务端的地址和协议
    InetAddress servaddr(ip, port);
    servsock->bind(servaddr); // 绑定ip和端口
    servsock->listen();       // 开启监听

    Channel *servchannel = new Channel(&loop_, servsock->fd());
    servchannel->setreadcallback(std::bind(&Channel::newconnection, servchannel, servsock));
    // 把ch加入epoll句柄中，监听读事件
    servchannel->enablereading();
}

TcpServer::~TcpServer()
{
}

// 开启循环监听
void TcpServer::start()
{
    loop_.run();
}
