#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include "InetAddress.h"
#include "Logger.h"

extern Logger &logger;

// 创建一个非阻塞的fd
int createnonblocking();

// Socket类
// 主管Socket的属性设置和连接
class Socket
{
private:
    const int fd_;   // 控制socket的文件描述符
    std::string ip_; // 如果是listenfd，存放服务端监听的fd，如果是客户端连接的fd，存放对端的fd
    uint16_t port_;  // 如果是listenfd，存放服务端监听的port，如果是客户端连接的fd，存放外部端口

public:
    Socket(int fd); // 传入一个已准备好的fd
    ~Socket();

    int fd() const; // 返回fd成员变量
    std::string ip() const;
    uint16_t port() const;
    void setipport(const std::string &ip, uint16_t port); // 设置端口和ip

    void setreuseaddr(bool on);  // 设置SO_REUSEADDR,允许重新绑定处于`time_wait`状态的地址。
    void setreuseport(bool on);  // 设置SO_REUSEPORT， 允许多个套接字绑定到同一端口
    void settcpnodelay(bool on); // 设置TCP_NODELAY 选项，是否禁用Nagle算法
    void setkeepalive(bool on);  // 设置SO_KEEPALIVE,（TCP周期性发送探测消息）

    void listen(int nn = 128);              // 监听函数，服务端的socket将调用此函数
    void bind(const InetAddress &servaddr); // 服务端socket调用，绑定fd和ip端口协议等属性
    int accept(InetAddress &clientaddr);    // 服务端的socket将调用此函数接受新的客户端连接
};

#endif // SOCKET_H_