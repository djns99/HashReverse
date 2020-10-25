#pragma once

#include "PipelineStage.h"
#include "Math.h"

class ReevaluatePopulation : public PipelineStage
{
public:
    void operator()( MemeticAlgorithm& algorithm,
                             Population& population) override
    {
        population.reevaluate(algorithm.getObjectiveFunction());
    }
};

class ParallelReevaluatePopulation : public PipelineStage
{
public:
    void operator()( MemeticAlgorithm& algorithm,
                             Population& population) override
    {
        auto& members = population.getMembers();
        auto& thread_pool = ThreadPool::getThreadPool();
        uint64_t num_parts = std::min(population.size(), thread_pool.getNumThreads());
        uint64_t num_per_part = Math::ceilDiv(members.size(), num_parts);
        thread_pool.run([&](uint64_t tid){
            for(uint64_t i = tid * num_per_part; i < (tid + 1) * num_per_part && i < members.size(); i++)
                members[i].second = algorithm.getObjectiveFunction()(members[i].first);
        }, num_parts);
        population.reheap();
    }
};