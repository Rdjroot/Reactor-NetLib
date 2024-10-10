#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <memory>
#include <ctime>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <thread>
#include <queue>
#include <condition_variable>

// 日志级别
enum class LogLevel
{
    INFO,
    WARNING,
    ERROR
};

class Logger
{
public:
    static Logger &getInstance();

    void log(LogLevel level, const std::string &&message); // 输出日志
    void setLogLevel(LogLevel level);                      // 设置日志级别
    void setOutputDirectory(const std::string &dir);       // 设置日志输出目录
    void checkLogRotation();                               // 检查日志文件是否需要轮换

    // 辅助函数，用于生成格式化的日志消息
    template <typename... Args>
    void logFormatted(LogLevel level, const char *format, Args &&...args);

private:
    Logger();
    ~Logger();

    std::ofstream logFile;           // 日志文件流
    LogLevel logLevel;               // 日志级别
    std::mutex logMutex;             // 互斥锁保护队列
    std::string logDirectory;        // 日志目录
    size_t maxFileSize;              // 最大文件大小
    std::string currentLogFileName;  // 当前日志文件名
    std::queue<std::string> logQueue; // 日志队列
    std::condition_variable cv;      // 条件变量
    bool exitLoggingThread;          // 线程退出标志
    std::thread logThread;           // 日志处理线程
    
    std::string getTimestamp();                     // 获取时间戳
    std::string logLevelToString(LogLevel level);   // 日志级别转换为字符串
    void openNewLogFile();                          // 打开新的日志文件
    std::string generateLogFileName();              // 生成日志文件名

    bool directoryExists(const std::string &path);  // 检查目录是否存在
    void createDirectory(const std::string &path);  // 创建日志目录
    std::string getExecutablePath();                // 获取当前程序路径
    void logThreadFunction();                       // 日志线程的工作函数
    void asyncLog(LogLevel level, const std::string& message); // 异步日志函数
};

// 模板函数的实现
template <typename... Args>
void Logger::logFormatted(LogLevel level, const char *format, Args &&...args)
{
    if (level < logLevel)
    {
        return;
    }

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
    asyncLog(level, buffer);
}

#endif // LOGGER_H
