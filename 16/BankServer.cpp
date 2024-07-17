#include "BankServer.h"

BankServer::BankServer(const std::string &ip, uint16_t port, int subthreadnum, int workthreadnum)
    : tcpserver_(ip, port, subthreadnum), threadpool_(workthreadnum, "WORKS")
{
    // 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数
    tcpserver_.setnewconnectioncb(std::bind(&BankServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.setcloseconnectioncb(std::bind(&BankServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&BankServer::HandleError, this, std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&BankServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&BankServer::HandleSendComplete, this, std::placeholders::_1));
    // tcpserver_.settimeoutcb(std::bind(&BankServer::HandleTimeOut, this, std::placeholders::_1));
}

BankServer::~BankServer()
{
}

// 启动服务
void BankServer::Start()
{
    tcpserver_.start();
}

// 停止工作线程和IO线程
void BankServer::Stop()
{
    // 停止工作线程
    threadpool_.stop();
    std::cout << "工作线程已停止" << std::endl;
    tcpserver_.stop();
}

void BankServer::HandleNewConnection(spConnection conn)
{
    // std::cout << "New Connection Come in." << std::endl;

    // 根据业务需求编写代码
    std::cout << Timestamp::now().tostring() << " accept client(fd=" << conn->fd()
              << ", ip=" << conn->ip() << ", port=" << conn->port() << ") ok. " << std::endl;
}

void BankServer::HandleClose(spConnection conn)
{
    std::cout << Timestamp::now().tostring() << " client(eventfd=" << conn->fd() << ") disconnected.\n";
    // std::cout << "BankServer conn closed." << std::endl;
    // 根据业务需求编写代码
}

void BankServer::HandleError(spConnection conn)
{
    std::cout << "client(eventfd=" << conn->fd() << ") error." << std::endl;
    // std::cout << "BankServer conn errored." << std::endl;

    // 根据业务需求编写代码
}

void BankServer::HandleMessage(spConnection conn, std::string &message)
{
    if (threadpool_.size() == 0)
    {
        // 如果没有工作线程，表示在IO线程中计算
        OnMessage(conn, message);
    }
    else
    {
        // 把业务添加到工作线程池的任务队列中
        threadpool_.addtask(std::bind(&BankServer::OnMessage, this, conn, message));
    }
}

void BankServer::OnMessage(spConnection conn, std::string &message)
{
    // std::cout << Timestamp::now().tostring() << " message (eventfd=" << conn->fd() << "):" << message << std::endl;
    message = "reply " + message;
    conn->send(message.data(), message.size()); // 把数据发送出去
}

void BankServer::HandleSendComplete(spConnection conn)
{
    // std::cout << "Message send complete." << std::endl;

    // 根据业务需求编写代码
}

void BankServer::HandleTimeOut(EventLoop *loop)
{
    // std::cout << "BankServer timeout." << std::endl;
    // 根据业务需求编写代码
}
