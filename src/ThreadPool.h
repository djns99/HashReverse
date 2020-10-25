#pragma once

#include <cstdint>
#include <functional>
#include <thread>
#include <atomic>
#include "CVQueue.h"

class ThreadPool
{
public:
    explicit ThreadPool( uint64_t num_threads );
    ~ThreadPool();

    void run( const std::function<void( uint64_t )>& function,
              uint64_t num_parts );

    static ThreadPool& getThreadPool(uint64_t num_threads = 0);

    uint64_t getNumThreads() const;

    void resize(uint64_t new_num_threads);
private:
    static uint64_t toActualThreads( uint64_t count );
    void threadFunction();
    void stop();
    void start(uint64_t num_threads);

    struct Handle
    {
        Handle( const std::function<void( uint64_t )>& work,
                uint64_t num_parts )
                : work(work)
                , num_to_submit(num_parts)
                , num_to_complete(num_parts)
        {

        }

        const std::function<void( uint64_t )>& work;

        std::atomic<uint64_t> num_to_submit;
        uint64_t num_to_complete;

        std::mutex completion_mutex;
        std::condition_variable completion_cv;

        void complete();

        void wait();
    };

    CVQueue<Handle*> work_queue;
    std::vector<std::thread> workers;
};



