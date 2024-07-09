/**
 * 一个简单的epoll服务端程序
 * 主要监听写事件
 *
 */
#include <iostream>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <cstdlib> // 使用 <cstdlib> 替代 <stdlib.h>，因为这是 C++ 标准库中的头文件
#include <ctime>   // 使用 <ctime> 替代 <time.h>，因为这是 C++ 标准库中的头文件
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h> // TCP_NODELAY需要包含这个头文件。
#include <functional>
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "usage: ./tcpepoll ip port\n";
        cout << "example: ./tcpepoll 192.168.8.128  5085\n";
        return -1;
    }

    // 创建非阻塞的监听socket
    Socket servsock(createnonblocking());

    // 设置一些提高性能的属性
    servsock.setkeepalive(true);
    servsock.setreuseaddr(true);
    servsock.setreuseport(true);
    servsock.settcpnodelay(true);

    // 服务端的地址和协议
    InetAddress servaddr(argv[1], atoi(argv[2]));
    servsock.bind(servaddr);    // 绑定ip和端口  
    servsock.listen();          // 开启监听

    EventLoop loop;    

    Channel *servchannel = new Channel(&loop, servsock.fd());
    servchannel->setreadcallback(std::bind(&Channel::newconnection,servchannel,&servsock));

    // 把ch加入epoll句柄中，监听读事件
    servchannel->enablereading();

    // 开启事件循环
    loop.run();

    return 0;
}