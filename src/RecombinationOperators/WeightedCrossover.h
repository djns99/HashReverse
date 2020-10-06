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

template<class URBG>
using WeightedBitCrossover = WeightedCrossover<URBG, BitCrossover>;

template<class URBG>
using WeightedTermCrossover = WeightedCrossover<URBG, TermCrossover>;