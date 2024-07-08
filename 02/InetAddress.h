#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

class InetAddress
{
private:
    sockaddr_in addr_;

    public:
    // 监听的fd初始化
    InetAddress(const std::string &ip, uint16_t port);
    // 连上来的客户端
    InetAddress(const sockaddr_in addr);

    ~InetAddress();

    // 返回字符串表示的ip地址
    const char *ip() const;

    // 返回整数表示的端口号
    uint16_t port() const;
    
    // 返回addr_成员函数转换的sockaddr结构体
    const sockaddr *addr() const;
};