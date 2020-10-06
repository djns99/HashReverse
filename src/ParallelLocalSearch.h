#pragma once

#include "HashFunction.h"
#include "ObjectiveFunction.h"
#include "CVQueue.h"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

enum class LocalSearchMode;

class ParallelLocalSearch
{
public:
    ParallelLocalSearch( uint64_t num_threads );

    ~ParallelLocalSearch();

    std::pair<HashFunction, uint64_t> operator()( const HashFunction& in ) const;

    void setObjectiveFunction(ObjectiveFunction& objective_function);
private:
    void threadFunction( uint64_t index );

    uint64_t improve( HashFunction& f,
                      uint64_t current_score,
                      uint64_t i );

    ObjectiveFunction* objective_function = nullptr;
    std::vector<std::thread> thread_pool;
    const uint64_t num_threads;

    struct HashResult
    {
        const HashFunction& input;
        HashFunction output;
        uint64_t score;
        std::mutex completion_mutex;
        std::condition_variable completion_cv;
        bool completed = false;
    };

    mutable CVQueue<HashResult*> input_queue;

    std::mutex sync_mutex;
    uint64_t sync_best_score;
    HashFunction sync_best_hash;
    pthread_barrier_t sync_barrier;
    std::atomic<bool> shutting_down{false};
};



