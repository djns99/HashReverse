#pragma once

#include "Population.h"

class MemeticAlgorithm;

class PipelineStage
{
public:
    virtual void operator()( MemeticAlgorithm&,
                             Population& ) = 0;
    virtual ~PipelineStage() = default;
};

#include "MemeticAlgorithm.h"