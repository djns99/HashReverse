#pragma once

#include "../PipelineStage.h"

class PassthroughSelection : public PipelineStage
{
    void operator()( MemeticAlgorithm&,
                     Population& ) override
    {
    }
};