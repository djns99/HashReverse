/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "ReconstructionStrategy.h"
#include <cassert>

class PureCommaOperator : public ReconstructionStrategy
{
public:
    Population operator()( Population& old_pop,
                           Population& new_pop ) override
    {
        assert(new_pop.size() >= old_pop.size());
        new_pop.resize(old_pop.size());
        return new_pop;
    }
};
