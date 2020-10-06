#include <iomanip>
#include "MemeticAlgorithm.h"

HashFunction MemeticAlgorithm::run( uint64_t population_size,
                                    const std::function<bool( uint64_t,
                                                              Population& )>& should_stop )
{
    pop_size = population_size;
    Population p = generateInitial(population_size);
    for ( uint64_t i = 0; !should_stop(i, p); i++ ) {
        Population old = p;
        Population new_pop = (*processing_pipeline)(*this, p);
        p = (*reconstruction_strategy)(old, new_pop);
        // printPopulation(p);
        assert(p.size() == population_size);
        if ( (*convergence_criterion)(p) ) {
            p = restartPopulation(p);
        }
        assert(p.size() == population_size);
        std::cout << i << ": " << p.best().second << "\t\t\r" << std::flush;
    }

    return (p.best().second < best.second) ? p.best().first : best.first;
}

Population MemeticAlgorithm::generateInitial( uint64_t size )
{
    Population p(size);
    for ( uint64_t i = 0; i < size; i++ ) {
        HashFunction f = (*generator)(*this);
        uint64_t score = (*objective_function)(f);
        p.add(f, score);
    }
    return p;
}

Population MemeticAlgorithm::restartPopulation( Population& population )
{
    std::cout << "Restart" << std::endl;
    const uint64_t num_to_preserve = pop_size * restart_preservation_percentage;

    if ( population.best().second < best.second )
        best = population.best();

    Population new_pop(pop_size);
    for ( uint64_t i = 0; i < num_to_preserve; i++ ) {
        auto head = population.popHead();
        new_pop.add(std::move(head.first), head.second);
    }

    for ( uint64_t i = num_to_preserve; i < pop_size; i++ ) {
        HashFunction f = (*generator)(*this);
        uint64_t score = (*objective_function)(f);
        new_pop.add(std::move(f), score);
    }
    population.resize(0);
    std::cout << "Restart complete" << std::endl;
    return new_pop;
}

void MemeticAlgorithm::printPopulation( const Population& pop ) const
{
    uint64_t mem = 0;
    std::cout << std::endl;
    for ( auto& func : pop.getMembers() ) {
        std::cout << ++mem << "," << func.second << ":";
        uint64_t input_mask_lo = ~0ull >> (64 - func.first.getInputBits() * 2);
        for ( auto& pla : func.first.getPLAs() )
            for ( auto& term : pla )
                printf(" %s0x%03lx", term.isNegated() ? "~" : "", term.value_lo & input_mask_lo);
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}
