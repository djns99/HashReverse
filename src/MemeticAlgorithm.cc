#include <iomanip>
#include "MemeticAlgorithm.h"

HashFunction MemeticAlgorithm::run( uint64_t population_size,
                                    const std::function<bool( uint64_t,
                                                              Population& )>& should_stop )
{
    pop_size = population_size;
    Population p = generateInitial(population_size);
    restarts = 0;
    for ( iterations = 0; !should_stop(iterations, p); iterations++ ) {
        Population old = p;
        Population new_pop = (*processing_pipeline)(*this, p);
        p = (*reconstruction_strategy)(old, new_pop);
        assert(p.isHeap());
        updateBest(p.best());
        // printPopulation(p);
        assert(p.size() == population_size);
        if ( (*convergence_criterion)(p) ) {
            restarts++;
            p = restartPopulation(p);
        }
        assert(p.size() == population_size);
        std::cerr << iterations << ": " << best.second << "(" << p.best().second << ")" << "\t\t\t\r" << std::flush;
    }
    assert(best.second != UINT64_MAX);
    std::cerr << iterations << ": " << best.second << "\t\t\t\t\t\r" << std::endl;

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
    updateBest(p.best());
    return p;
}

Population MemeticAlgorithm::restartPopulation( Population& population )
{
    std::cerr << "Restart" << std::endl;
    const uint64_t num_to_preserve = pop_size * restart_preservation_percentage;

    updateBest(population.best());

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
    return new_pop;
}

void MemeticAlgorithm::printPopulation( const Population& pop )
{
    uint64_t mem = 0;
    std::cout << std::endl;
    for ( auto& func : pop.getMembers() ) {
        std::cout << ++mem << "," << func.second << ":";
        uint64_t input_mask_lo = ~0ull >> (64 - func.first.getInputBits() * 2);
        for ( auto& pla : func.first.getPLAs() )
            for ( auto& term : pla )
                fprintf(stderr, " %s0x%03lx", term.isNegated() ? "~" : "", term.value_lo & input_mask_lo);
        std::cerr << std::endl;
    }
    std::cerr << std::endl;
    std::cerr << std::endl;
}

void MemeticAlgorithm::updateBest( const std::pair<HashFunction, uint64_t>& candidate )
{
    if ( candidate.second < best.second )
        best = candidate;
}

uint64_t MemeticAlgorithm::getIterations() const
{
    return iterations;
}

uint64_t MemeticAlgorithm::getRestarts() const
{
    return restarts;
}
