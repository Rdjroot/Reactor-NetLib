#include "EchoServer.h"

/**
 * 构造函数
 * ip  服务端ip
 * port 服务端端口
 * subthreadnum 子线程（主要用于IO，处理已连接的客户端报文传输与请求）
 * workthreanum 工作线程（处理业务内容）
 */
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

// 停止工作线程和IO线程
void EchoServer::Stop()
{
    // 停止工作线程
    threadpool_.stop();
    logger.log(LogLevel::WARNING, "工作线程已停止.");
    tcpserver_.stop();
}

// 新的客户端连接
void EchoServer::HandleNewConnection(spConnection conn)
{
    // std::cout << "New Connection Come in." << std::endl;

    // 根据业务需求编写代码
    logger.logFormatted(LogLevel::WARNING, "accept client(fd=%d, ip=%s, port=%d) ok.", conn->fd(), conn->ip().c_str(), conn->port());
}

// 客户端连接断开
void EchoServer::HandleClose(spConnection conn)
{
    logger.logFormatted(LogLevel::WARNING, "client(eventfd=%d) disconnected.", conn->fd());

    // std::cout << "EchoServer conn closed." << std::endl;
    // 根据业务需求编写代码
}

// 客户端连接出错
void EchoServer::HandleError(spConnection conn)
{
    logger.logFormatted(LogLevel::WARNING, "client(eventfd=%d) error.", conn->fd());
    // std::cout << "EchoServer conn errored." << std::endl;

    // 根据业务需求编写代码
}

// 处理客户端传上来的报文
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

// 数据发送完成后的业务内容
void EchoServer::HandleSendComplete(spConnection conn)
{
    // std::cout << "Message send complete." << std::endl;

    // 根据业务需求编写代码
}

// epoll_wait()超时，这里没有内容
void EchoServer::HandleTimeOut(EventLoop *loop)
{
    // std::cout << "EchoServer timeout." << std::endl;
    // 根据业务需求编写代码
}
