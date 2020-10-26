/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "../PipelineStage.h"

template<class URBG>
class TournamentSelection : public PipelineStage
{
public:
    TournamentSelection( double survivor_percentage,
                         uint64_t tournament_members,
                         URBG& urbg )
            : survivor_percentage(survivor_percentage)
            , tournament_members(tournament_members)
            , urbg(urbg)
    {

    }

    void operator()( MemeticAlgorithm& memetic_algorithm,
                     Population& population ) override
    {
        uint64_t target_size = std::ceil(population.size() * survivor_percentage);
        if ( target_size == population.size() - 1 )
            population.resize(target_size);
        if ( target_size == population.size() )
            return;
        Population new_pop(target_size);
        population.sort();
        auto& members = population.getMembers();

        for ( uint64_t i = 0; i < target_size; i++ ) {
            if(tournament_members >= members.size()) {
                new_pop.getMembers().insert(new_pop.getMembers().end(), std::move_iterator(members.begin()), std::move_iterator(members.begin() + (target_size - i)));
                new_pop.reheap();
                break;
            }
            std::uniform_int_distribution<uint64_t> dist(0, population.size() - 1);
            std::pair<HashFunction, uint64_t>* best = &members[dist(urbg)];
            for ( uint64_t j = 1; j < tournament_members; j++ ) {
                uint64_t index = dist(urbg);
                if ( members[index].second < best->second )
                    best = &members[index];
            }
            new_pop.add(std::move(best->first), best->second);
            *best = std::move(members.back());
            members.pop_back();
        }
        population = std::move(new_pop);
        assert(population.size() == target_size);
    }

private:
    const double survivor_percentage;
    const uint64_t tournament_members;
    URBG& urbg;
};