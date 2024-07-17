#ifndef THREADPOOL_H_
#define THREADPOOL_H_
#include <iostream>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
#include <sys/syscall.h>
#include <future>
#include <atomic>
#include <unistd.h>
#include <vector>
#include <queue>
#include <string>
#include <sstream>

class ThreadPool
{
private:
    std::vector<std::thread> threads_;            // 线程池中的线程
    std::queue<std::function<void()>> taskqueue_; // 任务队列
    std::mutex mutex_;                            // 任务队列用于同步的互斥锁
    std::condition_variable condition_;           // 任务队列同步的条件变脸
    std::atomic_bool stop_;                       // 在析构函数中，把stop_的值设为true，全部线程将退出
    const std::string threadtype_;                // 标记线程的种类："IO","WORKS"
    std::mutex output_mutex_;
public:
    // 在构造函数中奖启动threadnum个线程
    ThreadPool(size_t threadnum,const std::string& threadtype);

    // 把任务 添加到队列中
    void addtask(std::function<void()> task);

    // 获取线程池的大小
    size_t size();

    void stop();    

    ~ThreadPool();
};

#endif