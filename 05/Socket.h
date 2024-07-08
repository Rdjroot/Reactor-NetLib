#ifndef SOCKET_H_ 
#define SOCKET_H_ 

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include "InetAddress.h"
#include <iostream>

// 创建一个非阻塞的fd
int createnonblocking();

// Socket类
// 主管Socket的属性设置和连接
class Socket
{
private:
    const int fd_; // 控制socket的文件描述符

public:
    Socket(int fd); // 传入一个已准备好的fd
    ~Socket();

    int fd() const;              // 返回fd成员变量
    void setreuseaddr(bool on);  // 设置SO_REUSEADDR,允许重新绑定处于`time_wait`状态的地址。
    void setreuseport(bool on);  // 设置SO_REUSEPORT， 允许多个套接字绑定到同一端口
    void settcpnodelay(bool on); // 设置TCP_NODELAY 选项，是否禁用Nagle算法
    void setkeepalive(bool on);  // 设置SO_KEEPALIVE,（TCP周期性发送探测消息）

    void listen(int nn = 128);              // 监听函数
    void bind(const InetAddress &servaddr); // 服务端socket调用，绑定fd和ip端口协议等属性
    int accept(InetAddress &clientaddr);    // 接受新的客户端连接
};

#endif // SOCKET_H_ 