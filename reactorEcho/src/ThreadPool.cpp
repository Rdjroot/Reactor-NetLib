#include "ThreadPool.h"

//  g++ -o test ThreadPool.cpp -lpthread

ThreadPool::ThreadPool(size_t threadnum, const std::string &threadtype)
    : stop_(false), threadtype_(threadtype)
{
    for (int i = 0; i < threadnum; i++)
    {
        threads_.emplace_back([this]
                              {
            logger.logFormatted(LogLevel::INFO, "create %s threadID(%d).",threadtype_.c_str(),syscall(SYS_gettid));
            logger.logFormatted(LogLevel::WARNING,"create thread ,id is %d.", std::this_thread::get_id());
            // 打印出线程号，这里的线程号是操作系统分配的，可查
            // C++11自带的是this_thread::get_id() 不是同一个格式，这里暂不采用。
            // std::cout<<"create thread(" << syscall(SYS_gettid) << ")."<<std::endl;
            while(stop_ == false)
            {
                std::function<void()> task;     // 用于存放出队的元素

                { // 锁的作用域开始
                    std::unique_lock<std::mutex> lock(this->mutex_);

                    // 等待生产者的条件变量
                    this->condition_.wait(lock, [this]
                        {
                            return ((this->stop_ == true) || (!this->taskqueue_.empty()));
                        }
                    );

                    if((this->stop_==true) && (this->taskqueue_.empty()))
                        return;
                    
                    task = std::move(this->taskqueue_.front());
                    this->taskqueue_.pop();
                }

                // logger.logFormatted(LogLevel::WARNING, "(%d)thread execute task.",std::this_thread::get_id());
                task();     // 执行任务
            } });
    }
}

// 添加任务
void ThreadPool::addtask(std::function<void()> task)
{
    { //////上锁
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    } /////////释放锁

    condition_.notify_one(); // 唤醒一个线程，执行任务
}

size_t ThreadPool::size()
{
    return threads_.size();
}

// 停止线程
void ThreadPool::stop()
{
    if (stop_)
        return;
    stop_ = true;

    condition_.notify_all(); // 唤醒所有线程

    // 等待所有任务执行完析构
    for (std::thread &th : threads_)
    {
        auto id = th.get_id();
        th.join();
        // logger.logFormatted(LogLevel::WARNING, "delete thread ,id is %d.", id);
    }
}


ThreadPool::~ThreadPool()
{
    stop();
}
