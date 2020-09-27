#pragma once

#include "ConvergenceCriteria/ConvergenceCriterion.h"
#include "ObjectiveFunction.h"
#include "Pipeline.h"
#include "ReconstructionStrategies/ReconstructionStrategy.h"
#include "SolutionGenerators/SolutionGenerator.h"
#include <functional>
#include <memory>
#include <iostream>

class MemeticAlgorithm
{
public:
    explicit MemeticAlgorithm( std::unique_ptr<ObjectiveFunction> objective_function,
                               std::unique_ptr<SolutionGenerator> generator,
                               std::unique_ptr<Pipeline> update_pipeline,
                               std::unique_ptr<ReconstructionStrategy> reconstruction_strategy,
                               std::unique_ptr<ConvergenceCriterion> convergence_criterion,
                               double restart_preservation_percentage )
            : objective_function(std::move(objective_function))
            , generator(std::move(generator))
            , processing_pipeline(std::move(update_pipeline))
            , reconstruction_strategy(std::move(reconstruction_strategy))
            , convergence_criterion(std::move(convergence_criterion))
            , restart_preservation_percentage(restart_preservation_percentage)
            , best(HashFunction(0, 0, 0), UINT64_MAX)
    {
    }

    HashFunction run( uint64_t population_size,
                      const std::function<bool( uint64_t,
                                                uint64_t )>& should_stop )
    {
        pop_size = population_size;
        Population p = generateInitial(population_size);
        for ( uint64_t i = 0; !should_stop(i, p.best().second); i++ ) {
            Population old = p;
            Population new_pop = (*processing_pipeline)(*this, p);
            p = (*reconstruction_strategy)(old, new_pop);
            assert(p.size() == population_size);
            if ( (*convergence_criterion)(p) ) {
                std::cout << "Restart" << std::endl;
                p = restartPopulation(p);
            }
            assert(p.size() == population_size);
            std::cout << i << ": " << p.best().second << "\t\t\r" << std::flush;
        }

        return (p.best().second < best.second) ? p.best().first : best.first;
    }

    [[nodiscard]] ObjectiveFunction& getObjectiveFunction() const
    {
        return *objective_function;
    }

    [[nodiscard]] uint64_t getPopulationSize() const
    {
        return pop_size;
    }

private:
    Population generateInitial( uint64_t size );

    Population restartPopulation( Population& );

    std::unique_ptr<ObjectiveFunction> objective_function;
    std::unique_ptr<SolutionGenerator> generator;
    std::unique_ptr<Pipeline> processing_pipeline;
    std::unique_ptr<ReconstructionStrategy> reconstruction_strategy;
    std::unique_ptr<ConvergenceCriterion> convergence_criterion;
    double restart_preservation_percentage;
    uint64_t pop_size;

    std::pair<HashFunction, uint64_t> best;
};
