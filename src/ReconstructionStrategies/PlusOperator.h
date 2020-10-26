/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "ReconstructionStrategy.h"
#include <cassert>

class PurePlusOperator : public ReconstructionStrategy
{
public:
    Population operator()( Population& old_pop,
                           Population& new_pop ) override
    {
        uint64_t target_size = old_pop.size();
        Population combined(target_size);
        for ( uint64_t i = 0; i < target_size; i++ ) {
            if(new_pop.size() == 0) {
                auto& old_members = old_pop.getMembers();
                combined.getMembers().insert(combined.getMembers().end(), std::move_iterator(old_members.begin()), std::move_iterator(old_members.begin() + (target_size - i)));
                combined.reheap();
                break;
            }
            auto& best_old = old_pop.best();
            auto& best_new = new_pop.best();
            if ( best_old.second < best_new.second ) {
                auto head = old_pop.popHead();
                combined.add(std::move(head.first), head.second);
            } else {
                auto head = new_pop.popHead();
                combined.add(std::move(head.first), head.second);
            }
        }
        assert(combined.size() == target_size);
        return combined;
    }
};
