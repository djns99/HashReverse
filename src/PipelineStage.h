#pragma once
#include "Population.h"

class PipelineStage
{
public:
    virtual void operator()(Population&) = 0;
};