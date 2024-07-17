/**
 * 一个简单的客户端程序
 */
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <cstdlib> // 使用 <cstdlib> 替代 <stdlib.h>，因为这是 C++ 标准库中的头文件
#include <ctime>   // 使用 <ctime> 替代 <time.h>，因为这是 C++ 标准库中的头文件
#include <cstring>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

// 发送报文，支持4字节的报头
ssize_t tcpsend(int fd, void *data, size_t size)
{
    char tmpbuf[1024]; // 临时的buffer，报文头部+报文内容
    memset(tmpbuf, 0, sizeof(tmpbuf));
    memcpy(tmpbuf, &size, 4);       // 拼接报文头部
    memcpy(tmpbuf + 4, data, size); // 拼接报文内容

    return send(fd, tmpbuf, size + 4, 0); // 把请求报文发送给服务端
}

ssize_t tcprecv(int fd, void *data)
{
    int len;
    recv(fd, &len, 4, 0);          // 先读取4字节的报文头部
    return recv(fd, data, len, 0); // 读取报文内容
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "usage:./bankclient ip port\n";
        cout << "example:./bankclient 192.168.8.128  5085\n";
        return -1;
    }

    int sockfd;

    // 创建socket失败
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "socket() failed." << endl;
        return -1;
    }

    // C++11以上使用聚合初始化来初始化结构体的成员
    // 与memset(&servaddr,0,sizeof(servaddr))作用相等
    struct sockaddr_in servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(std::atoi(argv[2])); // 字符串 port -> int port(主机序) -> 网络字节序
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // 字符串 ip -> 大端序 ip

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        cout << "connect(" << argv[1] << ":" << argv[2] << ") failed." << endl;
        close(sockfd);
        return -1;
    }

    cout << "connect ok." << endl;
    cout << "开始时间：" << time(0) << endl;

    ///////////登录业务///////////////
    string buf = {};
    buf = "<bizcode>00101</bizcode><username>rdjroot</username><password>123456</password>";
    if (tcpsend(sockfd, buf.data(), buf.size()) <= 0)
    {
        cout << "tcpsend() failed" << endl;
        return -1;
    }
    cout << "发送： " << buf << endl;

    buf = {};
    if (tcprecv(sockfd, buf.data()) <= 0)
    {
        cout << "tcprecv() failed " << endl;
        return -1;
    }
    cout << "接收：" << buf << endl;

    cout << "结束时间：" << time(0) << endl;

    return 0;
}