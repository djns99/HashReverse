/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "../PipelineStage.h"

class PassthroughSelection : public PipelineStage
{
    void operator()( MemeticAlgorithm&,
                     Population& ) override
    {
    }
};