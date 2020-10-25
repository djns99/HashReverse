#pragma once

#include "MemeticAlgorithm.h"

class RandomWalk : public MemeticAlgorithm
{
public:
    template<class URBG>
    RandomWalk( std::unique_ptr<ObjectiveFunction> objective_function,
                         std::unique_ptr<SolutionGenerator> generator,
                         URBG& urbg,
                         bool parallel = false )
            : MemeticAlgorithm(std::move(objective_function),
                               std::move(generator),
                               buildPipeline(urbg),
                               std::make_unique<PureCommaOperator>(),
                               std::make_unique<AlwaysFalseConvergence>(),
                               0.0)
            , parallel(parallel)
    {

    }

    template<class URBG>
    std::unique_ptr<Pipeline> buildPipeline( URBG& urbg )
    {
        std::vector<std::unique_ptr<PipelineStage>> pipeline{};
        if ( parallel ) {
            pipeline.emplace_back(new ParallelUniformDistRandomMutation<URBG>(1.0f, urbg));
            pipeline.emplace_back(new ParallelReevaluatePopulation());
        } else {
            pipeline.emplace_back(new UniformDistRandomMutation<URBG>(1.0f, urbg));
            pipeline.emplace_back(new ReevaluatePopulation());
        }
        return std::make_unique<Pipeline>(std::move(pipeline));
    }

private:
    const bool parallel = false;
};