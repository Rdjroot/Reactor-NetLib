#include "InetAddress.h"

// 如果是用于监听的fd，用这个构造函数。
InetAddress::InetAddress(const std::string &ip, uint16_t port)
{
    addr_.sin_family = AF_INET;                                 // IPv4网络协议的套接字类型。
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());              // 服务端用于监听的ip地址。
    addr_.sin_port = htons(port);                               // 服务端用于监听的端口。
}

InetAddress::InetAddress(const sockaddr_in addr)
{
    addr_ = addr;
}

InetAddress::~InetAddress()
{
}

const char *InetAddress::ip() const
{
    // 大端序IP -> 字符串IP
    return inet_ntoa(addr_.sin_addr);
}

uint16_t InetAddress::port() const
{
    // 网络序端口 - > 主机序端口
    return ntohs(addr_.sin_port);
}

const sockaddr *InetAddress::addr() const
{
    return (sockaddr *)&addr_;
}

void InetAddress::setaddr(sockaddr_in clientaddr)
{
    addr_ = clientaddr;
}
