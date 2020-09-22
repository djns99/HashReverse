#pragma once
#include "PipelineStage.h"
#include "Population.h"
#include <vector>
#include <memory>
class Pipeline
{
public:
    explicit Pipeline(const std::vector<std::unique_ptr<PipelineStage>>& stages)
        : stages(stages)
    {
    }

    Population operator()(const Population& population) const
    {
        Population new_pop = population;
        for(auto& stage : stages)
            (*stage)(new_pop);
        return new_pop;
    }

private:
    const std::vector<std::unique_ptr<PipelineStage>>& stages;
};
