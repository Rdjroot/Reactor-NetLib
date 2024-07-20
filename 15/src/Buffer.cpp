#include "Buffer.h"

Buffer::Buffer(uint16_t sep) : sep_(sep)
{
}

Buffer::~Buffer()
{
}

void Buffer::append(const char *data, size_t size)
{
    buf_.append(data, size);
}

void Buffer::appendwithsep(const char *data, size_t size)
{
    if (sep_ == 0)
    {
        buf_.append(data, size); // 把报文内容加入
    }
    else if (sep_ == 1)
    {
        buf_.append((char *)&size, 4); // 处理报文长度
        buf_.append(data, size);       // 把报文内容加入
    }    
    else if (sep_ == 2)
    {
        buf_.append(data, size);       // 把报文内容加入
        buf_.append("\r\n\r\n", 4);    // 加入分隔符
    }
    // 补充书写sep_=2的情况
}

size_t Buffer::size()
{
    return buf_.size();
}

const char *Buffer::data()
{
    return buf_.data();
}

void Buffer::clear()
{
    buf_.clear();
}

void Buffer::erase(size_t pos, size_t sz)
{
    buf_.erase(pos, sz);
}

bool Buffer::pickmessge(std::string &ss)
{
    if (buf_.empty())
        return false;

    if (sep_ == 0)
    {
        ss = buf_;
    }
    else if (sep_ == 1)
    {
        // 取出报文首部
        int len = *reinterpret_cast<const int *>(buf_.data());

        // 如果inputbuffer_的数据量小于报文头部，说明inputbuffer_中的报文不完整
        if (buf_.size() < static_cast<size_t>(len + 4))
            return false;

        // 从inputbuffer中取出一个报文（略过报文头部）
        ss = buf_.substr(4, len);
        buf_.erase(0, len + 4); // 删除已经取出的数据
    }
    else if (sep_ == 2)
    {
        size_t pos = buf_.find("\r\n\r\n");

        // 如果没有找到分隔符，说明报文不完整
        if (pos == std::string::npos)
            return false;

        // 从buf_中取出一个报文
        ss = buf_.substr(0, pos);
        buf_.erase(0, pos + 4); // 删除已经取出的数据和分隔符
    }
    return true;
}
