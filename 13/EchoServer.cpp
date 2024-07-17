#include "EchoServer.h"

EchoServer::EchoServer(const std::string &ip, uint16_t port, int subthreadnum, int workthreadnum)
    : tcpserver_(ip, port, subthreadnum), threadpool_(workthreadnum, "WORKS")
{
    // 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数
    tcpserver_.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.setcloseconnectioncb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    // tcpserver_.settimeoutcb(std::bind(&EchoServer::HandleTimeOut, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{
}

// 启动服务
void EchoServer::Start()
{
    tcpserver_.start();
}

void EchoServer::HandleNewConnection(spConnection conn)
{
    std::cout << "New Connection Come in." << std::endl;

    // 根据业务需求编写代码
    std::cout << "accept client(fd=" << conn->fd() << ", ip=" << conn->ip()
              << ", port=" << conn->port() << ") ok." << std::endl;
}

void EchoServer::HandleClose(spConnection conn)
{
    std::cout << "client(eventfd=" << conn->fd() << ") disconnected.\n";
    std::cout << "EchoServer conn closed." << std::endl;
    // 根据业务需求编写代码
}

void EchoServer::HandleError(spConnection conn)
{
    std::cout << "client(eventfd=" << conn->fd() << ") error." << std::endl;
    std::cout << "EchoServer conn errored." << std::endl;

    // 根据业务需求编写代码
}

void EchoServer::HandleMessage(spConnection conn, std::string &message)
{
    if (threadpool_.size() == 0)
    {
        // 如果没有工作线程，表示在IO线程中计算
        OnMessage(conn, message);
    }
    else
    {
        // 把业务添加到工作线程池的任务队列中
        threadpool_.addtask(std::bind(&EchoServer::OnMessage, this, conn, message));
    }
}

void EchoServer::OnMessage(spConnection conn, std::string &message)
{
    message = "reply " + message;
    conn->send(message.data(), message.size()); // 把数据发送出去
}

void EchoServer::HandleSendComplete(spConnection conn)
{
    std::cout << "Message send complete." << std::endl;

    // 根据业务需求编写代码
}

void EchoServer::HandleTimeOut(EventLoop *loop)
{
    std::cout << "EchoServer timeout." << std::endl;
    // 根据业务需求编写代码
}
