#include "ThreadPool.h"

//  g++ -o test ThreadPool.cpp -lpthread

ThreadPool::ThreadPool(size_t threadnum,const std::string threadtype) 
    : stop_(false), threadtype_(threadtype)
{
    for (int i = 0; i < threadnum; i++)
    {
        threads_.emplace_back([this]
                              {
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
                std::cout << threadtype_ <<" thread execute task, the threadID is " << syscall(SYS_gettid)<< std::endl;
                task();     // 执行任务
            } });
    }
}

// 添加任务
void ThreadPool::addtask(std::function<void()> task)
{
    {   //////上锁
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    }/////////释放锁

    condition_.notify_one();    // 唤醒一个线程，执行任务
}

ThreadPool::~ThreadPool()
{
    stop_ = true;

    condition_.notify_all();            // 唤醒所有线程

    // 等待所有任务执行完析构
    for(std::thread &th : threads_)
        th.join();
}


// void show(int no, const std::string &name)
// {
//     printf("小哥哥们好，我是第%d号超级女生%s。\n",no,name.c_str());
// }

// void test()
// {
//     printf("我有一只小小鸟。\n");
// }

// int main()
// {
//     ThreadPool threadpool(3);
    
//     std::string name="西施";
//     threadpool.addtask(std::bind(show,8,name));
//     sleep(1);

//     threadpool.addtask(std::bind(test));
//     sleep(1);

//     threadpool.addtask(std::bind([]{ printf("我是一只傻傻鸟。\n");}));
//     sleep(1);
// }