#pragma once

#include "../RankDistribution.h"
#include "../PipelineStage.h"

template<class URBG>
class RankSelection : public PipelineStage
{
public:
    RankSelection( double survivor_percentage,
                         URBG& urbg )
            : survivor_percentage(survivor_percentage)
            , urbg(urbg)
    {

    }

    void operator()( MemeticAlgorithm& memetic_algorithm,
                     Population& population) override
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
            RankDistribution dist(members.size());
            uint64_t index = dist(urbg);
            auto& best = members[index];
            new_pop.add(std::move(best.first), best.second);
            members.erase(members.begin() + index);
        }
        population = std::move(new_pop);
        assert(population.size() == target_size);
    }

private:
    const double survivor_percentage;
    URBG& urbg;
};