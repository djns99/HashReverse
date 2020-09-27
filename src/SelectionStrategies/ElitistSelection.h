#pragma once

#include "../PipelineStage.h"

class ElitistSelection : public PipelineStage
{
public:
    ElitistSelection( double survivor_percentage )
            : survivor_percentage(survivor_percentage)
    {
    }

    void operator()( MemeticAlgorithm& mimetic_algorithm,
                     Population& population ) override
    {
        population.resize(std::ceil(population.size() * survivor_percentage));
    }

private:
    double survivor_percentage;
};