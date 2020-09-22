#pragma once
#include "ConvergenceCriterion.h"
#include "ObjectiveFunction.h"
#include "Pipeline.h"
#include "ReconstructionStrategy.h"
#include "SolutionGenerator.h"
#include <functional>
#include <memory>

class MimeticAlgorithm
{
public:
    explicit MimeticAlgorithm(std::unique_ptr<ObjectiveFunction> objective_function,
                              std::unique_ptr<SolutionGenerator> generator,
                              std::unique_ptr<Pipeline> update_pipeline,
                              std::unique_ptr<ReconstructionStrategy> reconstruction_strategy,
                              std::unique_ptr<ConvergenceCriterion> convergence_criterion,
                              double restart_preservation_percentage)
        : objective_function(std::move(objective_function))
        , generator(std::move(generator))
        , processing_pipeline(std::move(update_pipeline))
        , reconstruction_strategy(std::move(reconstruction_strategy))
        , convergence_criterion(std::move(convergence_criterion))
        , restart_preservation_percentage(restart_preservation_percentage)
    {
    }

    HashFunction run(uint64_t population_size, const std::function<bool(uint64_t)>& should_stop)
    {
        Population p = generateInitial(population_size);
        for (uint64_t i = 0; !should_stop(i); i++)
        {
            Population new_pop = (*processing_pipeline)(p);
            p = (*reconstruction_strategy)(p, new_pop);
            if ((*convergence_criterion)(p))
            {
                p = restartPopulation(p);
            }
        }

        return p.best().first;
    }

private:
    Population generateInitial(uint64_t size);
    Population restartPopulation(Population&);

    std::unique_ptr<ObjectiveFunction> objective_function;
    std::unique_ptr<SolutionGenerator> generator;
    std::unique_ptr<Pipeline> processing_pipeline;
    std::unique_ptr<ReconstructionStrategy> reconstruction_strategy;
    std::unique_ptr<ConvergenceCriterion> convergence_criterion;
    double restart_preservation_percentage;
};
