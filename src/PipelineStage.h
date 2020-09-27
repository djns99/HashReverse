#pragma once

#include "MemeticAlgorithm.h"
#include "Population.h"

class MemeticAlgorithm;

class PipelineStage
{
public:
    virtual void operator()( MemeticAlgorithm&,
                             Population& ) = 0;
};