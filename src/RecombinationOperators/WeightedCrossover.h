#pragma once

#pragma once

#include "../PipelineStage.h"
#include "CrossoverFunctions.h"
#include "../RankDistribution.h"
#include <random>

template<class URBG, class CrossoverFunction>
class WeightedCrossover : public PipelineStage
{
public:
    WeightedCrossover( double new_pop_ratio,
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

        RankDistribution<uint64_t> dist(old_size);
        while ( new_pop.size() < new_size ) {
            if ( old_size > 1 ) {
                const uint64_t first_idx = dist(urbg);
                const uint64_t second_idx = dist(urbg);
                if ( first_idx == second_idx )
                    continue;
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
class ParallelWeightedCrossover : public PipelineStage
{
public:
    ParallelWeightedCrossover( double new_pop_ratio,
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
            RankDistribution<uint64_t> dist(old_size);
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
using WeightedBitCrossover = WeightedCrossover<URBG, BitCrossover>;

template<class URBG>
using WeightedTermCrossover = WeightedCrossover<URBG, TermCrossover>;

template<class URBG>
using ParallelWeightedBitCrossover = ParallelWeightedCrossover<URBG, BitCrossover>;

template<class URBG>
using ParallelWeightedTermCrossover = ParallelWeightedCrossover<URBG, TermCrossover>;