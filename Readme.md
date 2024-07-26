# 基于Reactor模型的高并发网络库

## 项目简介

该项目是一个高性能仿muduo的网络库，采用Reactor主从模型设计，C++11标准编写，旨在提供高并发的网络服务。

通过使用epoll进行IO多路复用，结合线程池、定时器和智能指针等技术，实现了高效的事件分发和资源管理。

本网络库能够解耦网络和业务模块代码，便于扩展和维护。

![未命名文件](https://raw.githubusercontent.com/Rdjroot/Img_beds/master/img/202407251205543.png)

<center>主从Reactor模型</center>

## 主要特性

1. **高并发网络服务**：采用epoll进行IO多路复用，支持水平触发和边缘触发、支持高并发连接，保证了网络服务的高效性和稳定性。
2. **线程池**：实现了线程池管理，有效地分配和调度任务，提升程序性能。
3. **事件驱动架构**：通过Reactor主从模型，确保事件的快速响应和处理。
4. **智能指针管理**：使用智能指针管理资源，避免内存泄漏，提高代码的健壮性。
5. **定时器**：集成高精度定时器，支持定时任务的管理和调度。
6. **日志系统**：提供全面的日志记录和管理功能，便于问题定位和调试。
7. **扩展性**：模块化设计，支持业务逻辑的快速扩展和定制。

## 项目结构

```makefile
project_root/
├── include/
│   ├── InetAddress.h        # 地址协议类
│   ├── Socket.h             # 封装socket
│   ├── Epoll.h              # 封装epoll
│   ├── Channel.h            # 处理发生事件、回调
│   ├── EventLoop.h          # 事件循环类
│   ├── TcpServer.h          # 底层类，业务层到服务端的入口
│   ├── Logger.h             # 日志类
│   ├── Acceptor.h           # 服务端监听
│   ├── Connection.h         # 客户端连接
│   ├── Buffer.h             # 输入、输出缓冲区
│   ├── EchoServer.h         # 业务层代码，用于测试
│   ├── ThreadPool.h         # 线程池
│   └── Timestamp.h          # 时间戳封装
├── src/
│   ├── InetAddress.cpp
│   ├── Socket.cpp
│   ├── Epoll.cpp
│   ├── Channel.cpp
│   ├── EventLoop.cpp
│   ├── TcpServer.cpp
│   ├── Logger.cpp
│   ├── Acceptor.cpp
│   ├── Connection.cpp
│   ├── Buffer.cpp
│   ├── EchoServer.cpp
│   ├── ThreadPool.cpp
│   └── Timestamp.cpp
├── CMakeLists.txt
├── client.cpp               # 用于压力测试的客户端main程序
└── echoserver.cpp           # 用于开启服务端的main程序
```

## 模块说明

### 核心模块

- **InetAddress**: 网络地址封装类，处理IP地址和端口的封装与解析。
- **Socket**: 对底层socket接口进行封装，简化socket的创建、绑定、监听和连接操作。
- **Epoll**: 封装epoll系统调用，支持高效的IO事件多路复用。
- **Channel**: 管理文件描述符的事件和回调操作，负责事件的注册和分发。
- **EventLoop**: 事件循环类，核心的事件驱动机制，管理和调度各类事件。
- **TcpServer**: 服务器类，负责初始化服务器，接受新连接并处理数据。
- **Logger**: 日志管理类，提供多级日志记录和输出功能，便于调试和维护。
- **Acceptor**: 连接接受器，处理新客户端连接请求。
- **Connection**: 客户端连接类，管理客户端连接的状态和数据读写操作。
- **Buffer**: 数据缓冲区，提供高效的读写缓存管理。
- **EchoServer**: 业务层示例，实现简单的echo服务。
- **ThreadPool**: 线程池管理类，支持多线程任务调度和执行。
- **Timestamp**: 时间戳类，提供时间相关操作和格式化功能。

### 示例程序

- **client.cpp**: 压力测试客户端，用于模拟大量客户端连接和请求。
- **echoserver.cpp**: 服务器主程序，初始化并启动服务器，进入事件循环。

## 使用说明

> 实际运行结果请到项目的log目录下查看

### 依赖项

- CMake >= 3.0
- C++11及以上标准库
- Linux 环境

### 编译和安装

使用CMake构建项目：

```cpp
mkdir build
cd build
cmake ..
make
```

### 启动服务端

运行`echoserver`程序：

```cpp
// 服务端程序 服务端ip 监听窗口
./echoserver 192.168.8.128 5085
```

### 启动客户端

运行`client`程序进行测试：

```cpp
// 执行程序  服务端ip  服务端监听端口
./client 192.168.8.128 5085
    
// 可编写脚本在脚本中写入如下内容，写一百行，
// 即为100个客户端请求，总共百万报文
./client 192.168.8.128 5085&
    
// 启动脚本
sh tmp.sh
```

## 示例代码

### 服务端代码示例（echoserver.cpp）

> 可通过ctrl + C   或者 killall 命令终止程序

```cpp
#include <signal.h>
#include "EchoServer.h"
#include "Logger.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

EchoServer *echoserver;
Logger &logger = Logger::getInstance();

// 1、设置2和15的信号
// 2、在信号处理函数中停止主从时间循环和工作线程
// 3、服务程序主动退出
void Stop(int sig)
{
    logger.logFormatted(LogLevel::WARNING, "sig=%d", sig); // 输出是什么信号让其停止
    // 调用Echoserver的Stop()停止服务
    echoserver->Stop();
    logger.log(LogLevel::WARNING, "echoserver 已停止服务");
    delete echoserver;
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "usage: ./echoserver ip port\n";
        cout << "example: ./echoserver 192.168.8.128  5085\n";
        return -1;
    }

    signal(SIGTERM, Stop); // 信号15 系统kill或者killall命令
    signal(SIGINT, Stop);  // 信号2 Ctrl + C
    
    logger.setLogLevel(LogLevel::WARNING);
    // 服务端的地址和协议
    echoserver = new EchoServer(argv[1], atoi(argv[2]), 10, 0);

    logger.log(LogLevel::WARNING, "服务端程序开始启动。");

    // 开启事件循环
    echoserver->Start();

    return 0;
}
```

### 客户端代码示例（client.cpp）

> 单个客户端一次性发送一万条报文

```cpp
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

    // cout << "connect ok." << endl;
    // cout << "开始时间：" << time(0) << endl;
    // 接受/发送 的数据
    string buf(1024, '\0');

    for (int ii = 0; ii < 10000; ii++)
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
        recv(sockfd, &buf[0], len, 0);
        // cout << "recv: " << buf << endl;
        // sleep(1);
    }

    close(sockfd);
    // cout << "结束时间：" << time(0) << endl;

    return 0;
}
```

### 脚本

```bash
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
./client 192.168.8.128  5085&
```

## 自定义配置项

### 1. 日志分级配置

> 在主程序入口，设置最小输出日志等级

```cpp
// 有INFO WARNING ERROR三个等级
logger.setLogLevel(LogLevel::WARNING);
```

### 2. I/O线程和工作线程数配置

> 如果使用本程序中的应用层代码进行测试，则在主程序入口配置（方法一）
>
> 如果自行编写应用层代码，在底层入口TcpServer中进行设置（方法二）

```cpp
// 方法一：main函数中设置
// 四个参数分别为：ip、端口、IO线程数（事件循环线程）、工作线程数量
// 如果计算量小请设置工作线程为0，这样效率更高
echoserver = new EchoServer(argv[1], atoi(argv[2]), 10, 0);

// 方法二
// 应用层的tcpserver成员函数，仅可以设置IO线程数，工作线程数需要在应用层代码中自行定义。
tcpserver_(ip, port, subthreadnum)
```

### 3. 报文分隔方式

> Buffer支持三种报文分隔符：
>
> 1、代号：`0`-无分隔符(固定长度、视频会议)
>
> 2、代号： `1`-四字节的报头
>
> 3、代号：`2`-"\r\n\r\n"分隔符（http协议）
>
> 根据具体的使用方式，请在Connection中的Buffer中进行，设置，默认为`1`

```cpp
// Connection.h/cpp
// 如下所例，
Connection::Connection(args...):...inputbuffer_(2),outputbuffer_(2)...
```

### 4. 客户端连接空闲时间判定

> 在EventLoop设置，定时器默认相应间隔是30s，超时判断是80s

```cpp
// EventLoop.h/cpp
EventLoop(bool mainloop, int timeval = 30, int timeout = 80);
```

### 5. 客户端连接边缘/水平触发配置

> 在Conncetion的构造函数中设置，channel默认是水平触发，如果需要边缘触发需要单独设置。

```cpp
// 本项目中Connection默认采用边缘触发，不需要的话就删除构造函数中的边缘触发设置
Connection::Connection(args...)
    : ...
{
    ...
    clientchannel_->useet();         // 设置边缘触发，
	...
}
```