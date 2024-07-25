#include "Acceptor.h"

// 构造函数
// 创建非阻塞的监听socket
Acceptor::Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port)
    : loop_(loop), servsock_(createnonblocking()), acceptchannel_(loop_, servsock_.fd())
{
    // 设置一些提高性能的属性
    servsock_.setkeepalive(true);
    servsock_.setreuseaddr(true);
    servsock_.setreuseport(true);
    servsock_.settcpnodelay(true);
    // 服务端的地址和协议
    InetAddress servaddr(ip, port);
    servsock_.bind(servaddr); // 绑定ip和端口
    servsock_.listen();       // 开启监听

    acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection, this));

    acceptchannel_.enablereading(); // 把ch加入epoll句柄中，监听读事件
}

Acceptor::~Acceptor()
{
}

// 处理新客户端连接请求
void Acceptor::newconnection()
{
    InetAddress clientaddr; // 存放客户端的地址和协议

    // 返回新的客户端的连接，在accept中设置成了非阻塞的
    std::unique_ptr<Socket> clientsock(new Socket(servsock_.accept(clientaddr)));

    clientsock->setipport(clientaddr.ip(), clientaddr.port());

    newconnectioncb_(std::move(clientsock)); // 回调TcpServer::newconnection()
}

// 设置处理新客户端连接请求的回调函数
void Acceptor::setnewconnectioncb(std::function<void(std::unique_ptr<Socket>)> fn)
{
    newconnectioncb_ = fn;
}
