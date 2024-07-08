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
#include "Socket.h"
#include "InetAddress.h"
#include "Epoll.h"

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

    Socket servsock(createnonblocking());
    servsock.setkeepalive(true);
    servsock.setreuseaddr(true);
    servsock.setreuseport(true);
    servsock.settcpnodelay(true);

    // 服务端的地址和协议
    InetAddress servaddr(argv[1], atoi(argv[2]));
    servsock.bind(servaddr);
    servsock.listen();

    Epoll ep;
    ep.addfd(servsock.fd(), EPOLLIN);
    std::vector<epoll_event> evs;

    while (true) // 事件循环。
    {
        evs = ep.loop();

        // 如果infds>0，表示有事件发生的fd的数量。
        for (auto &ev: evs) // 遍历epoll返回的数组evs。
        {
            // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
            // EPOLLRDHUP 表示对端关闭连接或半关闭。
            if (ev.events & EPOLLRDHUP)
            {
                cout << "client(eventfd=" << ev.data.fd << ") disconnected.\n";
                // 关闭该客户端的fd。
                close(ev.data.fd);
            }
            //  EPOLLIN 普通数据  EPOLLPRI带外数据
            else if (ev.events & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
            {
                // 如果是listenfd有事件，表示有新的客户端连上来。
                if (ev.data.fd == servsock.fd())
                {
                    InetAddress clientaddr;

                    Socket *clientsock = new Socket(servsock.accept(clientaddr));

                    cout << "accept client(fd=" << clientsock->fd() << ", ip=" << clientaddr.ip() << ", port=" << clientaddr.port() << ") ok." << endl;

                    // 边缘触发。
                    ep.addfd(clientsock->fd(), EPOLLIN| EPOLLET);
                }
                else // 如果是客户端连接的fd有事件。
                {
                    string buffer(1024, '\0');
                    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
                    {
                        buffer.assign(buffer.size(), '\0');

                        // 从套接字中读数据
                        ssize_t nread = read(ev.data.fd, &buffer[0], sizeof(buffer));

                        // 成功的读取到了数据。
                        if (nread > 0)
                        {
                            // 把接收到的报文内容原封不动的发回去。
                            cout << "recv(eventfd=" << ev.data.fd << "):" << buffer << endl;
                            send(ev.data.fd, &buffer[0], buffer.size(), 0);
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
                            cout << "client(eventfd=" << ev.data.fd << ") disconnected." << endl;
                            close(ev.data.fd); // 关闭客户端的fd。
                            break;
                        }
                    }
                }
            }
            else if (ev.events & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
            {
            }
            else // 其它事件，都视为错误。
            {
                cout << "client(eventfd=" <<ev.data.fd << ") error." << endl;
                close(ev.data.fd); // 关闭客户端的fd。
            }
        }
    }

    return 0;
}