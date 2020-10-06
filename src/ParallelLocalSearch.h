#pragma once

#include "HashFunction.h"
#include "ObjectiveFunction.h"
#include "LocalSearch.h"
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

class ParallelLocalSearch
{
public:
    ParallelLocalSearch( ObjectiveFunction& function,
                         uint64_t num_threads);

    ~ParallelLocalSearch();

    std::pair<HashFunction, uint64_t> operator()( const HashFunction& in ) const;

private:
    void threadFunction(uint64_t index);
    uint64_t improve(HashFunction& f, uint64_t current_score, uint64_t i);

    ObjectiveFunction& objective_function;
    std::vector<std::thread> thread_pool;
    const uint64_t num_threads;

    mutable std::mutex work_mutex;
    mutable std::condition_variable work_cv;
    mutable std::condition_variable loop_cv;
    mutable bool has_hash = false;
    mutable bool processing = false;
    mutable HashFunction hash_function;
    mutable uint64_t best_score = 0;
    uint64_t num_running = 0;
    uint64_t num_complete = 0;

    volatile bool running = true;
};



