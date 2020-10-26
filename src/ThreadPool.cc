/**
 * Daniel Stokes 1331334
 */
#include <iostream>
#include "ThreadPool.h"

ThreadPool::ThreadPool( uint64_t num_threads )
        : work_queue(0)
{
    start(num_threads);
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::threadFunction()
{
    while ( true ) {
        auto* data = work_queue.pop();
        if ( !data )
            break;
        data->work(--data->num_to_submit);
        data->complete();
    }
}

void ThreadPool::run( const std::function<void( uint64_t )>& function,
                      uint64_t num_parts )
{
    Handle data{function, num_parts};
    for ( uint64_t i = 0; i < num_parts; i++ )
        work_queue.push(&data);
    data.wait();
}

ThreadPool& ThreadPool::getThreadPool( uint64_t num_threads )
{
    static ThreadPool tp(num_threads);
    return tp;
}

uint64_t ThreadPool::toActualThreads( uint64_t count )
{
    if ( count != 0 )
        return count;
    return std::thread::hardware_concurrency();
}

uint64_t ThreadPool::getNumThreads() const
{
    return workers.size();
}

void ThreadPool::resize( uint64_t num_threads )
{
    stop();
    start(num_threads);
}

void ThreadPool::stop()
{
    work_queue.shutdown();
    for ( auto& worker : workers )
        worker.join();
}

void ThreadPool::start( uint64_t num_threads )
{
    num_threads = toActualThreads(num_threads);
    std::cerr << "Starting thread pool with " << num_threads << " threads" << std::endl;
    work_queue.resize(num_threads * 2);
    work_queue.start();
    workers.clear();
    for ( uint64_t i = 0; i < num_threads; i++ )
        workers.emplace_back([&]()
                             {
                                 this->threadFunction();
                             });
}

void ThreadPool::Handle::complete()
{
    std::lock_guard<std::mutex> l(completion_mutex);
    if ( !--num_to_complete )
        // Under lock so handle is not cleaned up beneath us
        completion_cv.notify_all();
}

void ThreadPool::Handle::wait()
{
    std::unique_lock<std::mutex> l(completion_mutex);
    completion_cv.wait(l, [&]()
    { return num_to_complete == 0; });
}
