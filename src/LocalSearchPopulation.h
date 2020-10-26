/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "PipelineStage.h"
#include "LocalSearch.h"

class LocalSearchPopulation : public PipelineStage
{
public:
    explicit LocalSearchPopulation( LocalSearchMode mode,
                                    uint64_t num_threads = 1 )
            : search(mode, num_threads)
    {

    }

    void operator()( MemeticAlgorithm& memetic_algorithm,
                     Population& population ) override
    {
        search.setObjectiveFunction(memetic_algorithm.getObjectiveFunction());
        for ( auto& func : population.getMembers() )
            func = search(func.first);
    };

private:
    LocalSearch search;
};