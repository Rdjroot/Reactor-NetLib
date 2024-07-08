#ifndef INETADDRESS_H_
#define INETADDRESS_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

// 封装sockaddr_in的数据结构（含ip端口绑定、协议等）
class InetAddress
{
private:
    sockaddr_in addr_;

public:
    InetAddress();                                     // 默认构造函数
    InetAddress(const std::string &ip, uint16_t port); // 监听的fd初始化
    InetAddress(const sockaddr_in addr);               // 连上来的客户端
    ~InetAddress();

    const char *ip() const;               // 返回字符串表示的ip地址
    uint16_t port() const;                // 返回整数表示的端口号
    const sockaddr *addr() const;         // 返回addr_成员函数转换的sockaddr结构体
    void setaddr(sockaddr_in clientaddr); // 设置addr_成员的值
};

# endif