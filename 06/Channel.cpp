#include "Channel.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

Channel::Channel(Epoll *ep, int fd) : ep_(ep), fd_(fd)
{
}

Channel::~Channel()
{
    // 对于fd_和ep_，本类都只是使用它，它们不属于本类，所以不能销毁和关闭
}

int Channel::fd()
{
    return fd_;
}
// 判断Channel是否在红黑树上
bool Channel::inpoll()
{
    return inepoll_;
}

// 需要监视的事件
uint32_t Channel::events()
{
    return events_;
}

// 已发生的事件
uint32_t Channel::revents()
{
    return revents_;
}

// 采用边缘触发
void Channel::useet()
{
    events_ = events_ | EPOLLET;
}

// 监视读事件
void Channel::enablereading()
{
    events_|=EPOLLIN;
    ep_->updatechannel(this);
}

// 记录Channel已添加到epoll的红黑树中
void Channel::setinepoll()
{
    inepoll_ = true;
}

// 设置已发生的事件
void Channel::setrevents(uint32_t ev)
{
    revents_ = ev;
}

// 处理发生的事件
void Channel::handleevent()
{
    // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    // EPOLLRDHUP 表示对端关闭连接或半关闭。
    if (revents_ & EPOLLRDHUP)
    {
        cout << "client(eventfd=" << fd_ << ") disconnected.\n";
        // 关闭该客户端的fd。
        close(fd_);
    }
    //  EPOLLIN 普通数据  EPOLLPRI带外数据
    else if (revents_ & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
    {
        // 如果是新的连接，这里fd绑定的应该是newconnection
        // 如果fd是客户端，这里应该绑定的应该是 onmessage
        readcallback_();
    }
    else if (revents_ & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
    {
    }
    else // 其它事件，都视为错误。
    {
        cout << "client(eventfd=" << fd_ << ") error." << endl;
        close(fd_); // 关闭客户端的fd。
    }
}

void Channel::newconnection(Socket *servsock)
{
    InetAddress clientaddr;

    // 返回新的客户端的连接，在accept中设置成了非阻塞的
    Socket *clientsock = new Socket(servsock->accept(clientaddr));

    cout << "accept client(fd=" << clientsock->fd() << ", ip=" << clientaddr.ip() << ", port=" << clientaddr.port() << ") ok." << endl;

    // 为新客户端连接准备读事件和属性设置，并添加到epoll中。
    Channel *clientchannel = new Channel(ep_, clientsock->fd());
    // 绑定回调函数
    clientchannel->setreadcallback(std::bind(&Channel::onmessage, clientchannel));
    clientchannel->useet();         // 设置边缘触发，
    clientchannel->enablereading(); // 将新的客户端fd的读事件添加到epoll中
}

void Channel::onmessage()
{
    string buffer(1024, '\0');
    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {
        buffer.assign(buffer.size(), '\0');

        // 从套接字中读数据
        ssize_t nread = read(fd_, &buffer[0], sizeof(buffer));

        // 成功的读取到了数据。
        if (nread > 0)
        {
            // 把接收到的报文内容原封不动的发回去。
            cout << "recv(eventfd=" << fd_ << "):" << buffer << endl;
            send(fd_, &buffer[0], buffer.size(), 0);
        }
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {
            continue;
        }
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            break;
        }
        else if (nread == 0) // 客户端连接已断开。
        {
            cout << "client(eventfd=" << fd_ << ") disconnected." << endl;
            close(fd_); // 关闭客户端的fd。
            break;
        }
    }
}

void Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_ = fn;
}
