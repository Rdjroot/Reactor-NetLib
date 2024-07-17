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

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "usage:./client ip port\n";
        cout << "example:./client 192.168.8.128  5085\n";
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
    // 接受/发送 的数据
    string buf(1024, '\0');

    for (int ii = 0; ii < 100000; ii++)
    {
        buf = "这是第" + std::to_string(ii) + "个超级女生。";
        string tmpbuf;

        int len = buf.size();

        // 拼接报文头部，len是int型为四字节
        tmpbuf.append(reinterpret_cast<char *>(&len), sizeof(len));

        tmpbuf.append(buf);

        // 把输入的内容发送给服务端
        // len为报文长度
        if (send(sockfd, &tmpbuf[0], len + 4, 0) <= 0)
        {
            cout << "write() failed." << endl;
            close(sockfd);
            return -1;
        }

        // 先读取四字节的报文头部，获取报文长度
        recv(sockfd, &len, 4, 0);

        buf.assign(1024, '\0');
        // 读取报文内容
        recv(sockfd, buf.data(), len, 0);
        // cout << "recv: " << buf << endl;
        // sleep(1);
    }

    close(sockfd);
    cout << "结束时间：" << time(0) << endl;

    return 0;
}