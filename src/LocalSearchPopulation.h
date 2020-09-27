#pragma once
#include "PipelineStage.h"
#include "LocalSearch.h"

class LocalSearchPopulation : public PipelineStage
{
public:
    LocalSearchPopulation(LocalSearchMode mode)
    {

    }

    void operator()( MemeticAlgorithm& memetic_algorithm,
                     Population& population )
    {
        LocalSearch search(mode, memetic_algorithm.getObjectiveFunction());
        for(auto& func : population.getMembers())
        {
            func = search(func.first);
        }
    };

private:
    LocalSearchMode mode;
};