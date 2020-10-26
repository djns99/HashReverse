/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "../Population.h"

class ConvergenceCriterion
{
public:
    virtual bool operator()( const Population& ) = 0;
    virtual ~ConvergenceCriterion() = default;
};