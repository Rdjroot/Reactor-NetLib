#ifndef BUFFER_H_
#define BUFFER_H_
#include <string>
#include <cstring>
// 读写缓冲区类
class Buffer
{
private:
    std::string buf_;    // 用于存放数据
    const uint16_t sep_; // 报文的分隔符：0-无分隔符(固定长度、视频会议)；
                         // 1-四字节的报头；2-"\r\n\r\n"分隔符（http协议）。

public:
    Buffer(uint16_t sep = 1);
    ~Buffer();

    void append(const char *data, size_t size);        // 追加数据
    void appendwithsep(const char *data, size_t size); // 把数据追加到buf)中，附加报文分隔符
    size_t size();                                     // 返回buf_大小
    const char *data();                                // 返回buf_的首地址
    void clear();                                      // 清空buf_
    void erase(size_t pos, size_t sz);                 // 从pos开始删除sz个字符

    bool pickmessge(std::string &ss); // 从buf中拆分出一个报文，放在ss中，如果buf中没有报文，返回false
};

#endif