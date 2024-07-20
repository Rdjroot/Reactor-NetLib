#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <iostream>
#include <string>
#include "Logger.h"

extern Logger &logger;

// 时间戳类
class Timestamp
{
private:
    time_t secsinceepoch_; // 整数表示的事件（从1970到现在）

public:
    Timestamp();                     // 用当前时间初始化对象
    Timestamp(int64_t sesinceepoch); // 用一个整数表示的事件初始化对象

    static Timestamp now(); // 返回表示当前时间的Timestamp对象

    time_t toint() const;         // 返回整数表示的时间
    std::string tostring() const; // 返回字符串表示的时间，格式：yyyy-mm-dd hh24:mm:ss
};

#endif