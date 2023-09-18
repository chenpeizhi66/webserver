/**
 * @Author peizhic
 * 
 * reference material: https://github.com/mtrebi/thread-pool/tree/master
 * 
*/

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<thread>
#include<condition_variable>
#include<mutex>
#include<vector>
#include<queue>
#include<future>

class ThreadPool{
private:
    bool m_stop;
    std::vector<std::thread>m_thread;
    std::queue<std::function<void()>>tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;

public:
    explicit ThreadPool(size_t threadNumber):m_stop(false){
        for(size_t i=0;i<threadNumber;++i)
        {
            m_thread.emplace_back(
                [this](){
                    for(;;)
                    {
                        std::function<void()>task;
                        {
                            std::unique_lock<std::mutex> lk(m_mutex);
                            m_cv.wait(lk,[this](){ return m_stop||!tasks.empty();});
                            if(m_stop && tasks.empty()) return;
                            task=std::move(tasks.front());
                            tasks.pop();
                        }
                        task();
                    }
                }
            );
        }
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;

    ThreadPool & operator=(const ThreadPool &) = delete;
    ThreadPool & operator=(ThreadPool &&) = delete;

    ~ThreadPool(){
        {
            std::unique_lock<std::mutex>lk(m_mutex);
            m_stop=true;
        }
        m_cv.notify_all();
        for(auto& threads:m_thread)
        {
            threads.join();
        }
    }


    /**
     * add task into the queue
    */
    template<typename F,typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>{
        auto taskPtr=std::make_shared<std::packaged_task<decltype(f(args...))()>>(
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
        );
        {
            std::unique_lock<std::mutex>lk(m_mutex);
            if(m_stop) throw std::runtime_error("submit on stopped ThreadPool");
            tasks.emplace([taskPtr](){ (*taskPtr)(); });
        }
        m_cv.notify_one();
        return taskPtr->get_future();

    }
};

#endif

// low-efficience
// #include <list>
// #include <cstdio>
// #include <exception>
// #include <pthread.h>
// #include "../locker/locker.h"

// template<typename T> // T is the type of the task
// class threadpool {
// public:
//     threadpool(int thread_number = 8, int max_requests = 1000);
//     ~threadpool();
//     bool append(T* request);

// private:
//     static void* worker(void* arg);
//     void run();

// private:
//     int m_thread_number; // number of threads
//     pthread_t *m_threads; // thread pool list, size is the number of threads
//     int m_max_requests; // max number of requests in the request array
//     std::list<T*> m_workqueue; // request list
//     locker m_queuelocker; // mutex locker
//     sem m_queuestat; // to judge whether there exists a task to handle
//     bool m_stop; // terminate the thread
// };

// template<typename T>
// threadpool<T>::threadpool(int thread_number, int max_requests) : 
//     m_thread_number(thread_number), m_max_requests(max_requests), 
//     m_stop(false), m_threads(NULL) {

//     if (thread_number <= 0 || max_requests <= 0) {
//         throw std::exception();
//     }

//     m_threads = new pthread_t[m_thread_number];
//     if (!m_threads) {
//         throw std::exception();
//     }

//     // create thread_number threads and set them into detach
//     for (int i = 0; i < thread_number; i ++) {
//         // worker must be a static function, but how to use thread pool's parameter?
//         if (pthread_create(m_threads+i, NULL, worker, this) != 0) {
//             delete [] m_threads;
//             throw std::exception();
//         }

//         if (pthread_detach(m_threads[i])) {
//             delete [] m_threads;
//             throw std::exception();
//         }
//     }
// }

// template<typename T>
// threadpool<T>::~threadpool() {
//     delete [] m_threads;
//     m_stop = true;
// }

// template<typename T>
// bool threadpool<T>::append(T *request) {
//     m_queuelocker.lock();
//     if (m_workqueue.size() > m_max_requests) {
//         m_queuelocker.unlock();
//         return false;
//     }

//     m_workqueue.push_back(request);
//     m_queuelocker.unlock();
//     m_queuestat.post(); // add sem, to judege wait or run

//     return true;
// }

// template<typename T>
// void* threadpool<T>::worker(void* arg) {
//     threadpool* pool = (threadpool*) arg;
//     pool->run();
//     return pool;
// }

// template<typename T>
// void threadpool<T>::run() {
//     while (!m_stop) {
//         // take the request and process the request
//         m_queuestat.wait();
//         m_queuelocker.lock();
//         if (m_workqueue.empty()) {
//             // no request found!
//             m_queuelocker.unlock();
//             continue;
//         }

//         T* request = m_workqueue.front();
//         m_workqueue.pop_front();
//         m_queuelocker.unlock();

//         if (!request) {
//             continue;
//         }

//         // process is a func of request, 
//         // defined in the class of requset
//         request->process();
//     }
// }