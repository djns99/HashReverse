#pragma once

#include "ConvergenceCriterion.h"

class AlwaysFalseConvergence : public ConvergenceCriterion
{
    bool operator()( const Population& ) override
    {
        return false;
    }
};