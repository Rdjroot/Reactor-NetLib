#include "Logger.h"

Logger &Logger::getInstance()
{
    static Logger instance;
    return instance;
}

// 构造函数，初始化日志级别和最大文件大小
Logger::Logger() : logLevel(LogLevel::INFO), maxFileSize(10 * 1024 * 1024), exitLoggingThread(false)
{
    logDirectory = getExecutablePath() + "/../log";
    openNewLogFile();
    logThread = std::thread(&Logger::logThreadFunction, this); // 启动日志线程
}

Logger::~Logger()
{
    {
        std::lock_guard<std::mutex> guard(logMutex);
        exitLoggingThread = true;
        cv.notify_all();
    }
    logThread.join(); // 等待日志线程结束

    if (logFile.is_open())
    {
        logFile.close();
    }
}

// 记录日志
void Logger::log(LogLevel level, const std::string&& message)
{
    if (level >= logLevel)
    {
        asyncLog(level, message);
    }
}

// 设置日志输出级别
void Logger::setLogLevel(LogLevel level)
{
    logLevel = level;
}

// 设置日志输出目录
void Logger::setOutputDirectory(const std::string& dir) {
    std::lock_guard<std::mutex> guard(logMutex);
    logDirectory = dir;
    openNewLogFile();
}

// 检查日志文件是否需要轮换
void Logger::checkLogRotation() {
    if (logFile.is_open() && logFile.tellp() >= maxFileSize) {
        openNewLogFile();
    }
}

// 获取当前时间戳
std::string Logger::getTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* now_tm = std::localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", now_tm);
    return std::string(buffer);
}

// 将日志级别转换为字符串
std::string Logger::logLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

// 打开新的日志文件
void Logger::openNewLogFile() {
    if (logFile.is_open()) {
        logFile.close();
    }

    if (!directoryExists(logDirectory)) {
        createDirectory(logDirectory);
    }

    currentLogFileName = generateLogFileName();
    logFile.open(currentLogFileName, std::ios_base::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << currentLogFileName << std::endl;
    }
}

// 生成日志文件名
std::string Logger::generateLogFileName() {
    std::time_t now = std::time(nullptr);
    std::tm* now_tm = std::localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", now_tm);
    std::ostringstream oss;
    oss << logDirectory << "/application_" << buffer << ".log";
    return oss.str();
}

// 检查目录是否存在
bool Logger::directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    } else if (info.st_mode & S_IFDIR) {
        return true;
    } else {
        return false;
    }
}

// 创建目录
void Logger::createDirectory(const std::string& path) {
    if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        std::cerr << "Failed to create directory: " << path << std::endl;
    }
}

std::string Logger::getExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    std::string exePath;
    if (count != -1) {
        exePath = std::string(result, count);
        exePath = exePath.substr(0, exePath.find_last_of('/'));
    }
    return exePath;
}

// 日志线程的工作函数
void Logger::logThreadFunction()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(logMutex);
        cv.wait(lock, [this] { return !logQueue.empty() || exitLoggingThread; });

        while (!logQueue.empty())
        {
            checkLogRotation();
            if (logFile.is_open())
            {
                logFile << logQueue.front();
                logFile.flush(); // 确保立即刷新
            }
            logQueue.pop();
        }

        if (exitLoggingThread && logQueue.empty())
        {
            break;
        }
    }
}

// 异步日志函数，将日志放入队列
void Logger::asyncLog(LogLevel level, const std::string& message)
{
    std::ostringstream logStream;
    logStream << getTimestamp() << " [" << logLevelToString(level) << "] " << message << std::endl;

    {
        std::lock_guard<std::mutex> guard(logMutex);
        logQueue.push(logStream.str());
    }
    cv.notify_one(); // 通知日志线程有新的日志
}
