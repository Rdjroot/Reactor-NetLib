/**
 * 一个简单的epoll服务端程序
 * 主要监听写事件
 *
 */
#include"TcpServer.h"

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

    cout << "hello world" <<endl;
    cout << argv[1] <<endl;
    cout << argv[2] <<endl;
    // 服务端的地址和协议
    TcpServer tcpserv(argv[1], atoi(argv[2]));

    // 开启事件循环
    tcpserv.start();

    return 0;
}