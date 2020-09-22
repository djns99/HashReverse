#pragma once
#include "Population.h"

class ReconstructionStrategy
{
public:
    virtual Population operator()(const Population& old_pop, const Population& new_pop) = 0;
};
