
#include "MimeticAlgorithm.h"
Population MimeticAlgorithm::generateInitial(uint64_t size)
{
    Population p(size);
    for(uint64_t i = 0; i < size; i++)
    {
        HashFunction f = (*generator)();
        uint64_t score = (*objective_function)(f);
        p.add(f, score);
    }
    return p;
}

Population MimeticAlgorithm::restartPopulation(Population& population)
{
    const uint64_t target_size = population.size();
    const uint64_t num_to_preserve = population.size() * restart_preservation_percentage;

    Population new_pop(target_size);
    for(uint64_t i = 0; i < num_to_preserve; i++)
    {
        auto head = population.popHead();
        new_pop.add(head.first, head.second);
    }

    for(uint64_t i = num_to_preserve; i < population.size(); i++)
    {
        HashFunction f = (*generator)();
        uint64_t score = (*objective_function)(f);
        new_pop.add(f, score);
    }
    return new_pop;
}
