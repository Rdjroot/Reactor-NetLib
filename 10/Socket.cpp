#include "Socket.h"

using std::cerr;
using std::endl;

// 创建非阻塞的监听socket
int createnonblocking()
{
    // 创建服务端用于监听的listenfd。
    // 添加`SOCK_NONBLOCK`属性，让listenfd变为非阻塞的。
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0)
    {
        cerr << "socket() failed" << endl;
    }
    return listenfd;
}

Socket::Socket(int fd) : fd_(fd){};

Socket::~Socket()
{
    close(fd_);
}

// 返回fd成员
int Socket::fd() const
{
    return fd_;
}

std::string Socket::ip() const
{
    return ip_;
}

uint16_t Socket::port() const
{
    return port_;
}

void Socket::setipport(const std::string &ip, uint16_t port)
{
    ip_ = ip;
    port_ = port;
}

// 禁用Nagle算法，立即发送小数据包
// 对实时性要求高的应用中使用
// 在本模型中必须包含
void Socket::settcpnodelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

// 允许重新绑定处于`time_wait`状态的地址。
// 也就是无视服务器连接断开的2MSL
// 在本模型中必须包含
void Socket::setreuseaddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

// 允许多个套接字绑定到同一个端口
// 多个进程可以监听同一个端口
// 有用，但意义不大
void Socket::setreuseport(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

// 启用 TCP 保持活动连接检测。TCP会周期性发送探测消息，以确保连接时活动的
// 可能有用，但建议自己做心跳
void Socket::setkeepalive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

// 将结构体和监听fd绑定
void Socket::bind(const InetAddress &servaddr)
{
    if (::bind(fd_, servaddr.addr(), sizeof(servaddr)) < 0)
    {
        cerr << "bind() failed" << endl;
        close(fd_);
        exit(-1);
    }
    
    setipport(servaddr.ip(), servaddr.port());
}

void Socket::listen(int nn)
{
    if (::listen(fd_, nn) != 0) // 在高并发的网络服务器中，第二个参数要大一些。
    {
        cerr << "listen() failed" << endl;
        close(fd_);
        exit(-1);
    }
}

int Socket::accept(InetAddress &clientaddr)
{
    sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);

    // accept4()函数是Linux 2.6.28之后新增的函数，用于替代accept()函数。
    // 添加SOCK_NONBLOCK，让clientfd变为非阻塞的。
    int clientfd = accept4(fd_, (struct sockaddr *)&peeraddr, &len, SOCK_NONBLOCK);

    clientaddr.setaddr(peeraddr); // 客户端
    return clientfd;
}