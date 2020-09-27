#include "MemeticAlgorithm.h"

Population MemeticAlgorithm::generateInitial( uint64_t size )
{
    Population p(size);
    for ( uint64_t i = 0; i < size; i++ ) {
        HashFunction f = (*generator)();
        uint64_t score = (*objective_function)(f);
        p.add(f, score);
    }
    return p;
}

Population MemeticAlgorithm::restartPopulation( Population& population )
{
    const uint64_t num_to_preserve = pop_size * restart_preservation_percentage;

    if ( population.best().second < best.second )
        best = population.best();

    Population new_pop(pop_size);
    for ( uint64_t i = 0; i < num_to_preserve; i++ ) {
        auto head = population.popHead();
        new_pop.add(std::move(head.first), head.second);
    }

    for ( uint64_t i = num_to_preserve; i < pop_size; i++ ) {
        HashFunction f = (*generator)();
        uint64_t score = (*objective_function)(f);
        new_pop.add(std::move(f), score);
    }
    population.resize(0);
    return new_pop;
}
