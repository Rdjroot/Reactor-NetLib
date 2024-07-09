#include "Acceptor.h"

Acceptor::Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port) : loop_(loop)
{
    // 创建非阻塞的监听socket
    servsock_ = new Socket(createnonblocking());

    // 设置一些提高性能的属性
    servsock_->setkeepalive(true);
    servsock_->setreuseaddr(true);
    servsock_->setreuseport(true);
    servsock_->settcpnodelay(true);

    // 服务端的地址和协议
    InetAddress servaddr(ip, port);
    servsock_->bind(servaddr); // 绑定ip和端口
    servsock_->listen();       // 开启监听

    acceptchannel_ = new Channel(loop_, servsock_->fd());
    acceptchannel_->setreadcallback(std::bind(&Acceptor::newconnection, this));
    // 把ch加入epoll句柄中，监听读事件
    acceptchannel_->enablereading();
}

Acceptor::~Acceptor()
{
    delete servsock_;
    delete acceptchannel_;
}

void Acceptor::newconnection()
{
    InetAddress clientaddr;

    // 返回新的客户端的连接，在accept中设置成了非阻塞的
    Socket *clientsock = new Socket(servsock_->accept(clientaddr));
    // std::cout << "00000accept  ip=" << clientaddr->ip()
    //           << ", port=" << clientaddr->port() << ") ok." << std::endl;
    // std::cout << "1111accept client(fd=" << clientsock->fd() << ", ip=" << clientsock->ip()
    //           << ", port=" << clientsock->port() << ") ok." << std::endl;
    newconnectioncb_(clientsock);
}

void Acceptor::setnewconnectioncb(std::function<void(Socket *)> fn)
{
    newconnectioncb_ = fn;
}
