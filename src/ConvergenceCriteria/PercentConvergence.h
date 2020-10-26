/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "ConvergenceCriterion.h"
#include <unordered_set>

class PercentConvergence : public ConvergenceCriterion
{
public:
    PercentConvergence(double percentage01)
        : percentage01(percentage01)
    {
    }

    bool operator()( const Population& pop ) override
    {
        if(pop.size() == 1)
            return false;
        const auto& members = pop.getMembers();
        std::unordered_set<uint64_t> hash_set;
        uint64_t target_size = pop.size() * (1.0-percentage01);
        for ( auto& item : members ) {
            if(hash_set.emplace(item.first.hashcode()).second && hash_set.size() >= target_size)
                return false;
        }
        return true;
    }

private:
    double percentage01;
};