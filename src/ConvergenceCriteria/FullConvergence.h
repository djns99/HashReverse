#pragma once

#include "ConvergenceCriterion.h"

class FullConvergence : public ConvergenceCriterion
{
public:
    bool operator()( const Population& pop ) override
    {
        if(pop.size() == 1)
            return false;
        const auto& members = pop.getMembers();
        for ( uint64_t i = 1; i < members.size(); i++ )
            if ( members[0] != members[i] )
                return false;
        return true;
    }
};