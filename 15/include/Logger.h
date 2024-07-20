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
#include <limits.h> // 添加此头文件

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
    void setLogLevel(LogLevel level);                      // 设置什么等级以上的日志才输出
    void setOutputDirectory(const std::string &dir);       // 设置输出日志的目录
    void checkLogRotation();                               // 检查日志文件是否需要轮换（满了）

    // 辅助函数，用于生成格式化的日志消息
    template <typename... Args>
    void logFormatted(LogLevel level, const char *format, Args &&...args);

private:
    Logger();
    ~Logger();

    std::ofstream logFile;          // 日志文件输出流
    LogLevel logLevel;              // 当前日志级别
    std::mutex logMutex;            // 写入互斥锁
    std::string logDirectory;       // 日志目录
    size_t maxFileSize;             // 单个日志文件的最大大小
    std::string currentLogFileName; // 当前日志文件名

    std::string getTimestamp();                    // 获取当前时间戳
    std::string logLevelToString(LogLevel level);  // 将日志级别转换为字符串
    void openNewLogFile();                         // 打开新的日志文件
    std::string generateLogFileName();             // 生成日志文件名
    bool directoryExists(const std::string &path); // 检查目录是否存在
    void createDirectory(const std::string &path); // 创建目录
    std::string getExecutablePath();               // 获取当前目录
};

template<typename... Args>
void Logger::logFormatted(LogLevel level, const char* format, Args&&... args) {
    if (level < logLevel) {
        return;
    }

    std::lock_guard<std::mutex> guard(logMutex);
    checkLogRotation();

    // 计算格式化后的字符串长度
    constexpr size_t bufferSize = 1024;
    char buffer[bufferSize];
    int len = snprintf(buffer, bufferSize, format, std::forward<Args>(args)...);
    if (len < 0 || len >= bufferSize) {
        throw std::runtime_error("Log message is too long");
    }

    std::ostringstream logStream;
    logStream << getTimestamp() << " [" << logLevelToString(level) << "] " << buffer << std::endl;

    if (logFile.is_open()) {
        logFile << logStream.str();
        logFile.flush();  // 确保立即刷新缓冲区
    } else {
        std::cerr << "Failed to write to log file: " << currentLogFileName << std::endl;
        std::cout << logStream.str();
    }
}

#endif // LOGGER_H
