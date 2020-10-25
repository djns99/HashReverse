#pragma once

#include "PipelineStage.h"
#include "Population.h"
#include <memory>
#include <vector>

class MemeticAlgorithm;

class Pipeline
{
public:
    explicit Pipeline( std::vector<std::unique_ptr<PipelineStage>>&& stages )
            : stages(std::move(stages))
    {
    }

    Population operator()( MemeticAlgorithm& algorithm,
                           const Population& population ) const
    {
        Population new_pop = population;
        for ( auto& stage : stages ) {
            (*stage)(algorithm, new_pop);
        }
        return new_pop;
    }

private:
    const std::vector<std::unique_ptr<PipelineStage>> stages;
};
