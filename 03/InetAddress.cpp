#include "InetAddress.h"

InetAddress::InetAddress()
{
}

// 如果是用于监听的fd，用这个构造函数。
InetAddress::InetAddress(const std::string &ip, uint16_t port)
{
    addr_.sin_family = AF_INET;                    // IPv4网络协议的套接字类型。
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); // 服务端用于监听的ip地址。
    addr_.sin_port = htons(port);                  // 服务端用于监听的端口。
}

// 连上来的客户端
InetAddress::InetAddress(const sockaddr_in addr)
{
    addr_ = addr;
}

InetAddress::~InetAddress()
{
}

// 返回字符串表示的ip地址
const char *InetAddress::ip() const
{
    // 大端序IP -> 字符串IP
    return inet_ntoa(addr_.sin_addr);
}

// 返回整数表示的端口号
uint16_t InetAddress::port() const
{
    // 网络序端口 - > 主机序端口
    return ntohs(addr_.sin_port);
}

// 返回addr_成员函数转换的sockaddr结构体
const sockaddr *InetAddress::addr() const
{
    return (sockaddr *)&addr_;
}

// 设置addr_成员的值
void InetAddress::setaddr(sockaddr_in clientaddr)
{
    addr_ = clientaddr;
}
