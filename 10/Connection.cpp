#include "Connection.h"

Connection::Connection(EventLoop *loop, Socket *clientsock) : loop_(loop), clientsock_(clientsock)
{
    // 为新客户端连接准备读事件和属性设置，并添加到epoll中。
    clientchannel_ = new Channel(loop_, clientsock_->fd());
    // 绑定回调函数
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage, this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));
    clientchannel_->useet();         // 设置边缘触发，
    clientchannel_->enablereading(); // 将新的客户端fd的读事件添加到epoll中
}

Connection::~Connection()
{
    delete clientsock_;
    delete clientchannel_;
}

int Connection::fd() const
{
    return clientsock_->fd();
}

std::string Connection::ip() const
{
    return clientsock_->ip();
}

uint16_t Connection::port() const
{
    return clientsock_->port();
}

void Connection::onmessage()
{
    std::string buffer(1024, '\0');
    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {
        buffer.assign(buffer.size(), '\0');

        // 从套接字中读数据
        ssize_t nread = ::read(fd(), &buffer[0], sizeof(buffer));

        // 成功的读取到了数据。
        if (nread > 0)
        {
            // 把接收到的报文内容原封不动的发回去。
            // std::cout << "recv(eventfd=" << fd() << "):" << buffer << std::endl;
            // send(fd(), &buffer[0], buffer.size(), 0);
            inputbuffer_.append(buffer.data(),nread);
        }
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {
            continue;
        }
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            std::cout << "recv(eventfd=" << fd() << "):" << inputbuffer_.data() << std::endl;
            // 假设这里进行了复杂运算
            outputbuffer_ = inputbuffer_;
            inputbuffer_.clear();                   // 清理接收缓冲区
            send(fd(), outputbuffer_.data(), outputbuffer_.size(),0);
            break;
        }
        else if (nread == 0) // 客户端连接已断开。
        {
            closecallback();
            break;
        }
    }
}

void Connection::closecallback()
{
    closecallback_(this);
}

void Connection::errorcallback()
{
    errorcallback_(this);
}

void Connection::setclosecallback(std::function<void(Connection *)> fn)
{
    closecallback_ = fn;
}

void Connection::seterrorcallback(std::function<void(Connection *)> fn)
{
    errorcallback_ = fn;
}
