#pragma once

#include "PipelineStage.h"

class ReevaluatePopulation : public PipelineStage
{
public:
    void operator()( MemeticAlgorithm& algorithm,
                             Population& population) override
    {
        population.reevaluate(algorithm.getObjectiveFunction());
    }
};