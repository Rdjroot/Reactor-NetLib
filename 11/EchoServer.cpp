#include "EchoServer.h"

EchoServer::EchoServer(const std::string &ip, uint16_t port) : tcpserver_(ip, port)
{
}

EchoServer::~EchoServer()
{
}

// 启动服务
void EchoServer::Start()
{
    tcpserver_.start();
}

void EchoServer::HandleNewConnection(Socket *clientsock)
{
    std::cout << "New Connection Come in." << std::endl;

    // 根据业务需求编写代码
}

void EchoServer::HandleClose(Connection *conn)
{
    std::cout << "EchoServer conn closed." << std::endl;

    // 根据业务需求编写代码
}

void EchoServer::HandleError(Connection *conn)
{
    std::cout << "EchoServer conn errored." << std::endl;

    // 根据业务需求编写代码
}

void EchoServer::HandleMessage(Connection *conn, std::string message)
{
    // 假设这里进行了复杂的计算
    message = "reply " + message;
    int len = message.size();            // 计算回应报文的大小
    std::string tmpbuf((char *)&len, 4); // 把报文头部填充到回应报文中。
    tmpbuf.append(message);
    conn->send(tmpbuf.data(), tmpbuf.size());
}

void EchoServer::HandleSendComplete()
{
    std::cout << "Message send complete." << std::endl;

    // 根据业务需求编写代码
}

void EchoServer::HandleTimeOut(EventLoop *loop)
{
    std::cout << "EchoServer timeout." << std::endl;

    // 根据业务需求编写代码
}
