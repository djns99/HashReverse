#pragma once

#include "../PipelineStage.h"
#include "CrossoverFunctions.h"
#include "../ThreadPool.h"
#include <random>

template<class URBG, class CrossoverFunction>
class SimpleCrossover : public PipelineStage
{
public:
    SimpleCrossover( double new_pop_ratio,
                     URBG& urbg )
            : new_pop_ratio(new_pop_ratio)
            , urbg(urbg)
    {
    }

    void operator()( MemeticAlgorithm& algorithm,
                     Population& old_pop ) override
    {
        const uint64_t old_size = old_pop.size();
        assert(old_size != 0);
        const uint64_t new_size = algorithm.getPopulationSize() * new_pop_ratio;
        auto& old_functions = old_pop.getMembers();
        Population new_pop(new_size);
        std::uniform_int_distribution<uint64_t> dist(0, old_size - 1);
        while ( new_pop.size() < new_size ) {
            if ( old_size > 1 ) {
                const uint64_t first_idx = dist(urbg);
                const uint64_t second_idx = dist(urbg);
                if ( first_idx == second_idx ) {
                    continue;
                }
                auto new_func = CrossoverFunction::crossover(old_functions[first_idx].first, old_functions[second_idx].first, urbg);
                new_pop.add(std::move(new_func), UINT64_MAX);
            } else {
                new_pop.add(old_functions[0].first, old_functions[0].second);
            }
        }
        old_pop = std::move(new_pop);
    }

private:
    double new_pop_ratio;
    URBG& urbg;
};

template<class URBG, class CrossoverFunction>
class ParallelSimpleCrossover : public PipelineStage
{
public:
    ParallelSimpleCrossover( double new_pop_ratio,
                             URBG& urbg )
            : new_pop_ratio(new_pop_ratio)
            , urbg(urbg)
    {
    }

    void operator()( MemeticAlgorithm& algorithm,
                     Population& old_pop ) override
    {
        const uint64_t old_size = old_pop.size();
        assert(old_size != 0);
        const uint64_t new_size = algorithm.getPopulationSize() * new_pop_ratio;
        ThreadPool& thread_pool = ThreadPool::getThreadPool();
        uint64_t num_parts = std::min(thread_pool.getNumThreads(), new_size);
        uint64_t num_per_part = (new_size + num_parts - 1) / num_parts;
        auto& old_functions = old_pop.getMembers();
        Population new_pop(new_size);
        auto& new_members = new_pop.getMembers();
        new_members.resize(new_size);

        auto function = [&]( uint64_t part )
        {
            std::uniform_int_distribution<uint64_t> dist(0, old_size - 1);
            for ( uint64_t i = part * num_per_part; i < (part + 1) * num_per_part && i < new_size; i++ ) {
                if ( old_size > 1 ) {
                    const uint64_t first_idx = dist(urbg);
                    const uint64_t second_idx = dist(urbg);
                    if ( first_idx == second_idx ) {
                        i--;
                        continue;
                    }
                    auto new_func = CrossoverFunction::crossover(old_functions[first_idx].first,
                                                                 old_functions[second_idx].first,
                                                                 urbg);
                    new_members[i] = {std::move(new_func), UINT64_MAX};
                } else {
                    new_members[i] = {old_functions[0].first, old_functions[0].second};
                }
            }
        };
        thread_pool.run(function, num_parts);
        old_pop = std::move(new_pop);
    }

private:
    double new_pop_ratio;
    URBG& urbg;
};


template<class URBG>
using SimpleBitCrossover = SimpleCrossover<URBG, BitCrossover>;

template<class URBG>
using SimpleTermCrossover = SimpleCrossover<URBG, TermCrossover>;


template<class URBG> using ParallelSimpleBitCrossover = ParallelSimpleCrossover<URBG, BitCrossover>;

template<class URBG> using ParallelSimpleTermCrossover = ParallelSimpleCrossover<URBG, TermCrossover>;