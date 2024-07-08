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
#include "InetAddress.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "usage: ./tcpepoll ip port\n"
             << endl;
        cout << "example: ./tcpepoll 192.168.8.128  5085\n\n";
        return -1;
    }

    // 创建服务端用于监听的listenfd。
    // 添加`SOCK_NONBLOCK`属性，让listenfd变为非阻塞的。
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0)
    {
        cerr << "socket() failed" << endl;
        return -1;
    }

    int opt = 1;
    // 允许重新绑定处于`time_wait`状态的地址。
    // 也就是无视服务器连接断开的2MSL
    // 在本模型中必须包含
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
    // 禁用Nagle算法，立即发送小数据包
    // 对实时性要求高的应用中使用
    // 在本模型中必须包含
    setsockopt(listenfd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));
    // 允许多个套接字绑定到同一个端口
    // 多个进程可以监听同一个端口
    // 有用，但意义不大
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    // 启用 TCP 保持活动连接检测。TCP会周期性发送探测消息，以确保连接时活动的
    // 可能有用，但建议自己做心跳
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));

    InetAddress servaddr(argv[1], atoi(argv[2]));

    // 将结构体和listenfd绑定
    if (bind(listenfd, servaddr.addr(), sizeof(servaddr)) < 0)
    {
        cerr << "bind() failed" << endl;
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 128) != 0) // 在高并发的网络服务器中，第二个参数要大一些。
    {
        cerr << "listen() failed" << endl;
        close(listenfd);
        return -1;
    }

    int epollfd = epoll_create(1); // 创建epoll句柄（红黑树）。

    // 为服务端的listenfd准备读事件。
    epoll_event ev; // 声明事件的数据结构。
    ev.data.fd = listenfd; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events = EPOLLIN;   // 让epoll监视listenfd的读事件，采用水平触发。

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev); // 把需要监视的listenfd和它的事件加入epollfd中。

    epoll_event evs[10]; // 存放epoll_wait()返回事件的数组。

    while (true) // 事件循环。
    {
        // 等待监视的fd有事件发生。
        // -1 表示需要无限等待
        int infds = epoll_wait(epollfd, evs, 10, -1);

        // 返回失败。
        if (infds < 0)
        {
            cerr << "epoll_wait() failed" << endl;
            break;
        }

        // 超时。
        if (infds == 0)
        {
            cerr << "epoll_wait() timeout." << endl;
            continue;
        }

        // 如果infds>0，表示有事件发生的fd的数量。
        for (int ii = 0; ii < infds; ii++) // 遍历epoll返回的数组evs。
        {
            // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
            // EPOLLRDHUP 表示对端关闭连接或半关闭。
            if (evs[ii].events & EPOLLRDHUP)
            {
                cout << "1client(eventfd=" << evs[ii].data.fd << ") disconnected.\n";
                // 关闭该客户端的fd。
                close(evs[ii].data.fd);
            }
            //  EPOLLIN 普通数据  EPOLLPRI带外数据
            else if (evs[ii].events & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
            {
                // 如果是listenfd有事件，表示有新的客户端连上来。
                if (evs[ii].data.fd == listenfd)
                {
                    sockaddr_in peeraddr;
                    socklen_t len = sizeof(peeraddr);

                    // accept4()函数是Linux 2.6.28之后新增的函数，用于替代accept()函数。
                    // 添加SOCK_NONBLOCK，让clientfd变为非阻塞的。
                    int clientfd = accept4(listenfd, (struct sockaddr *)&peeraddr, &len, SOCK_NONBLOCK);

                    cout << "accept client(fd=" << clientfd << ", ip=" << inet_ntoa(peeraddr.sin_addr) << ", port=" << ntohs(peeraddr.sin_port) << ") ok." << endl;

                    // 为新客户端连接准备读事件，并添加到epoll中。
                    ev.data.fd = clientfd;
                    ev.events = EPOLLIN | EPOLLET; // 边缘触发。
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
                }
                else // 如果是客户端连接的fd有事件。
                {
                    string buffer(1024, '\0');
                    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
                    {
                        buffer.assign(buffer.size(), '\0');

                        // 从套接字中读数据
                        ssize_t nread = read(evs[ii].data.fd, &buffer[0], sizeof(buffer));

                        // 成功的读取到了数据。
                        if (nread > 0)
                        {
                            // 把接收到的报文内容原封不动的发回去。
                            cout << "recv(eventfd=" << evs[ii].data.fd << "):" << buffer << endl;
                            send(evs[ii].data.fd, &buffer[0], buffer.size(), 0);
                        }
                        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
                        {
                            continue;
                        }
                        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
                        {
                            break;
                        }
                        else if (nread == 0) // 客户端连接已断开。
                        {
                            cout << "2client(eventfd=" << evs[ii].data.fd << ") disconnected." << endl;
                            close(evs[ii].data.fd); // 关闭客户端的fd。
                            break;
                        }
                    }
                }
            }
            else if (evs[ii].events & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
            {
            }
            else // 其它事件，都视为错误。
            {
                cout << "2client(eventfd=" << evs[ii].data.fd << ") error." << endl;
                close(evs[ii].data.fd); // 关闭客户端的fd。
            }
        }
    }

    return 0;
}