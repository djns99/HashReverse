#pragma once

#include "../Population.h"

class ReconstructionStrategy
{
public:
    virtual Population operator()( Population& old_pop,
                                   Population& new_pop ) = 0;
    virtual ~ReconstructionStrategy() = default;
};
