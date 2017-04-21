// 
// ThreadPool.hpp
// ThreadPool
// 
// Original Author: Jakob Progsch, Václav Zeman
// Modified By:     https://www.shiyanlou.com
// Original Link:   https://github.com/progschj/ThreadPool
//

#ifndef ThreadPool_hpp
#define ThreadPool_hpp
#include <vector>               // std::vector
#include <queue>                // std::queue
#include <memory>               // std::make_shared
#include <stdexcept>            // std::runtime_error
#include <thread>               // std::thread
#include <mutex>                // std::mutex,        std::unique_lock
#include <condition_variable>   // std::condition_variable
#include <future>               // std::future,       std::packaged_task
#include <functional>           // std::function,     std::bind
#include <utility>              // std::move,         std::forward

class ThreadPool {
public:
    //构造函数负责启动一定数量的工作线程
    inline ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0;i<threads;++i)
            //此处lambda表达式捕获this，即线程池实例
            workers.emplace_back([this] {
                //循环避免虚假唤醒
                for(;;)
                {
                    //定义函数对象的容器，存储任意的返回类型为void，参数表为空的函数
                    std::function<void()> task;
                    //临界区
                    {
                        //创建互斥锁
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        //阻塞当前线程，直到condition_variable被唤醒
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });

                        //如果当前线程已经结束且等待任务队列为空,则应直接返回
                        if(this->stop && this->tasks.empty())
                            return;
                        //否则让任务队列的队首任务作为需要执行的任务出队
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
    }

    //添加一个新的线程到线程池中
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
        //推导任务返回类型
        using return_type = typename std::result_of<F(Args...)>::type;
        //获得当前任务
        auto task = std::make_shared< std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            //禁止在线程池停止后加入新的线程
            if(stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            //将线程添加到执行任务队列中
            tasks.emplace([task]{ (*task)(); });
        }
        //通知一个正在等待的线程
        condition.notify_one();
        return res;
    }

    //销毁所有线程池中创建的线程
    inline ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        //通知所有等待线程
        condition.notify_all();
        //使所有异步线程转为同步执行
        for(std::thread &worker: workers)
            worker.join();
    }
private:
    std::vector<std::thread> workers;
    //任务队列
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;     //互斥锁
    std::condition_variable condition;      //互斥条件变量
    
    bool stop;
};
#endif /* ThreadPool_hpp */