#include "Connection.h"

// 构造函数
// 为新客户端连接准备读事件和属性设置，并添加到epoll中。
Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock)
    : loop_(loop), clientsock_(std::move(clientsock)),
      clientchannel_(new Channel(loop_, clientsock_->fd())), disconnect_(false)
{
    // 绑定回调函数
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage, this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));
    clientchannel_->setwritecallback(std::bind(&Connection::writecallback, this));
    // clientchannel_->useet();         // 设置边缘触发，
    clientchannel_->enablereading(); // 将新的客户端fd的读事件添加到epoll中
}

Connection::~Connection()
{
    // std::cout << "conn已析构" << std::endl;
    logger.logFormatted(LogLevel::WARNING, "Connection is over . fd is: %d", fd());
}

// 返回客户端的fd
int Connection::fd() const
{
    return clientsock_->fd();
}

// 返回客户端的ip
std::string Connection::ip() const
{
    return clientsock_->ip();
}

// 返回客户端的port
uint16_t Connection::port() const
{
    return clientsock_->port();
}

void Connection::onmessage()
{
    char buffer[1024];
    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {
        bzero(&buffer, sizeof(buffer));
        // 从套接字中读数据
        ssize_t nread = ::read(fd(), buffer, sizeof(buffer));

        // 成功的读取到了数据。
        if (nread > 0)
        {
            // 把读取的数据追加到接收缓冲区中
            inputbuffer_.append(buffer, nread);
        }
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {
            continue;
        }
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            std::string message;
            // 从接收缓冲区中拆分出客户端的请求消息。
            while (true)
            {
                // 如果无法再拆分出报文
                if (!inputbuffer_.pickmessge(message))
                    break;

                lastime_ = Timestamp::now(); // 更新时间戳

                try
                {
                    // 将拆出的报文进行计算，并返回数据
                    onmessagecallback_(shared_from_this(), message); // 回调TcpServer::onmessage()处理客户端的请求消息
                }
                catch (const std::exception &e)
                {
                    logger.logFormatted(LogLevel::ERROR, "ON MESSAGE CALL BACK Error: %s, fd() is: %d", e.what(), fd());
                }
            }
            break;
        }
        else if (nread == 0) // 客户端连接已断开。
        {
            closecallback();
            break;
        }
    }
}

// TCP连接关闭（断开）的回调函数，供Channel回调
void Connection::closecallback()
{
    disconnect_ = true;
    clientchannel_->remove(); // 从红黑树上卸掉该socketfd
    closecallback_(shared_from_this());
}

// TCP连接错误的回调函数，供Channel回调
void Connection::errorcallback()
{
    disconnect_ = true;
    clientchannel_->remove(); // 从事件循环中删除该channel
    errorcallback_(shared_from_this());
}

// 处理写事件的回调函数，供Channel回调
void Connection::writecallback()
{
    // 尝试把发送缓冲区的数据全部发出去
    int writen = ::send(fd(), outputbuffer_.data(), outputbuffer_.size(), 0);
    if (writen > 0)
        outputbuffer_.erase(0, writen); // 从outputbuffer_中删除已成功发送的字节数

    // 如果数据已经全部发送，不再关注写事件
    if (outputbuffer_.size() == 0)
    {
        clientchannel_->disablewriting();
        sendcompletecallback_(shared_from_this());
    }
}

// 设置关闭fd_的回调函数
void Connection::setclosecallback(std::function<void(spConnection)> fn)
{
    closecallback_ = fn; // 回调TcpServer::closeconnection()
}

// 设置fd_发生了错误的回调函数
void Connection::seterrorcallback(std::function<void(spConnection)> fn)
{
    errorcallback_ = fn; // 回调TcpServer::errorconnection()
}

// 设置处理报文的回调函数
void Connection::setonmessagecallback(std::function<void(spConnection, std::string &)> fn)
{
    onmessagecallback_ = fn; // 回调TcpServer::onmessage()
}

// 发送数据完成后的回调函数
void Connection::setsendcompletecallback(std::function<void(spConnection)> fn)
{
    sendcompletecallback_ = fn;
}

// 发送数据（任何线程都是调用此函数）
void Connection::send(const char *data, size_t sz)
{
    if (disconnect_)
    {
        logger.log(LogLevel::WARNING, "客户端连接已断开。。。send()直接返回。");
        return;
    }
    try
    {
        // 因为数据要发送给其它线程处理，所以，把它包装成智能指针
        std::shared_ptr<std::string> message = std::make_shared<std::string>(data);

        // 判断当前线程是否为事件循环线程（IO线程）
        if (loop_->isinloopthread())
        {
            // 如果是IO线程，直接执行发送数据的操作
            // logger.log(LogLevel::INFO,"send()在事件循环(IO)的线程中" );
            sendinloop(message);
        }
        else
        {
            // 如果当前线程不是IO线程，调用EventLoop::queueinloop()，把sendinloop()交给事件循环线程去执行
            // logger.log(LogLevel::INFO,"send()不在事件循环(IO)的线程中" );
            loop_->queueinloop(std::bind(&Connection::sendinloop, this, message));
        }
    }
    catch (const std::exception &e)
    {
        logger.logFormatted(LogLevel::ERROR, "SEND TO THREAD Error: %s", e.what());
    }
}

// 写数据放入输出缓冲区，注册写事件
void Connection::sendinloop(std::shared_ptr<std::string> data)
{
    // 把数据存到缓冲区
    outputbuffer_.appendwithsep(data->data(), data->size());
    clientchannel_->enablewriting(); // 注册写事件
}

bool Connection::timeout(time_t now, int val)
{
    // 秒数大于val就算超时
    return now - lastime_.toint() > val;
}