#ifndef BUFFER_H_
#define BUFFER_H_
#include <string>
#include <iostream>

class Buffer
{
private:
    std::string buf_;      // 用于存放数据
public:
    Buffer();
    ~Buffer();

    void append(const char *data, size_t size);         // 追加数据
    size_t size();  // 返回buf_大小
    const char *data(); // 返回buf_的首地址
    void clear();       // 清空buf_
};

#endif