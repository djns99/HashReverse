#pragma once

#include "MemeticAlgorithm.h"

class RandomWalk
{
public:
    template<class URBG>
    explicit RandomWalk( std::unique_ptr<ObjectiveFunction> objective_function,
                         std::unique_ptr<SolutionGenerator> generator,
                         URBG& urbg)
            : algorithm(std::move(objective_function),
                        std::move(generator),
                        buildPipeline(urbg),
                        std::make_unique<PureCommaOperator>(),
                        std::make_unique<AlwaysFalseConvergence>(),
                        0.0)
    {

    }

    template<class URBG>
    std::unique_ptr<Pipeline> buildPipeline(URBG& urbg)
    {
        pipeline.clear();
        pipeline.emplace_back(new UniformDistRandomMutation<URBG>(1.0f, urbg));
        pipeline.emplace_back(new ReevaluatePopulation());
        return std::make_unique<Pipeline>(pipeline);
    }

    HashFunction run( const std::function<bool( uint64_t,
                                                Population& )>& should_stop )
    {
        return algorithm.run(100, should_stop);
    }

private:
    std::vector<std::unique_ptr<PipelineStage>> pipeline;
    MemeticAlgorithm algorithm;
};