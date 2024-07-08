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
#include "Socket.h"

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

    Socket servsock(createnonblocking());
    servsock.setkeepalive(true);
    servsock.setreuseaddr(true);
    servsock.setreuseport(true);
    servsock.settcpnodelay(true);

    // 服务端的地址和协议
    InetAddress servaddr(argv[1], atoi(argv[2]));
    servsock.bind(servaddr);
    servsock.listen();

    int epollfd = epoll_create(1); // 创建epoll句柄（红黑树）。

    // 为服务端的listenfd准备读事件。
    epoll_event ev;        // 声明事件的数据结构。
    ev.data.fd = servsock.fd(); // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events = EPOLLIN;   // 让epoll监视listenfd的读事件，采用水平触发。

    epoll_ctl(epollfd, EPOLL_CTL_ADD, servsock.fd(), &ev); // 把需要监视的listenfd和它的事件加入epollfd中。

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
                if (evs[ii].data.fd == servsock.fd())
                {
                    InetAddress clientaddr;

                    Socket *clientsock = new Socket(servsock.accept(clientaddr));

                    cout << "accept client(fd=" << clientsock->fd() << ", ip=" << clientaddr.ip() << ", port=" << clientaddr.port() << ") ok." << endl;

                    // 为新客户端连接准备读事件，并添加到epoll中。
                    ev.data.fd = clientsock->fd();
                    ev.events = EPOLLIN | EPOLLET; // 边缘触发。
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock->fd(), &ev);
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