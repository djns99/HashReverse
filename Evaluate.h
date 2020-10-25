#pragma once

#include "src/HashFunction.h"
#include "src/LocalSearch.h"
#include "src/MemeticAlgorithm.h"
#include "src/ObjectiveFunction.h"
#include "src/ReconstructionStrategies/CommaOperator.h"
#include "src/ReconstructionStrategies/PlusOperator.h"
#include "src/SolutionGenerators/LocalSearchGenerator.h"
#include "src/SolutionGenerators/RandomGenerator.h"
#include "src/RecombinationOperators/SimpleCrossover.h"
#include "src/RecombinationOperators/WeightedCrossover.h"
#include "src/MutationStrategies/RandomMutation.h"
#include "src/ConvergenceCriteria/AlwaysFalseConvergence.h"
#include "src/ConvergenceCriteria/PercentConvergence.h"
#include "src/ConvergenceCriteria/FullConvergence.h"
#include "src/SelectionStrategies/PassthroughSelection.h"
#include "src/SelectionStrategies/ElitistSelection.h"
#include "src/SelectionStrategies/RankSelection.h"
#include "src/SelectionStrategies/TournamentSelection.h"
#include "src/LocalSearchPopulation.h"
#include "src/ReevaluatePopulation.h"
#include "src/RandomWalk.h"
#include "src/FileReaderObjectiveFunction.h"

#include <iostream>

#ifndef CSV
#define CSV false
#endif
constexpr bool csv = CSV;

template<class URBG, bool local_search>
std::unique_ptr<MemeticAlgorithm> createSingleThreadedAlgorithm( uint64_t num_inputs,
                                                                 uint64_t num_outputs,
                                                                 uint64_t max_terms,
                                                                 ObjectiveFunction& objective_function,
                                                                 URBG& urbg )
{

    std::vector<std::unique_ptr<PipelineStage>> pipeline;
    // Selection strategy
    // pipeline.emplace_back(new RankSelection<URBG>(local_search ? 0.25 : 0.6, urbg));
    pipeline.emplace_back(new TournamentSelection<URBG>(local_search ? 0.25 : 0.6, 5, urbg));
    // pipeline.emplace_back(new ElitistSelection(0.6, urbg));
    // pipeline.emplace_back(new PassthroughSelection());

    // Local Search pipeline stage - done here cause the population is the smallest
    if ( local_search )
        pipeline.emplace_back(new LocalSearchPopulation(LocalSearchMode::FIRST_IMPROVEMENT));
    // pipeline.emplace_back(new LocalSearchPopulation(LocalSearchMode::BEST_IMPROVEMENT));

    // Crossover strategy
    // pipeline.emplace_back(new SimpleTermCrossover<URBG>(6, urbg));
    // pipeline.emplace_back(new SimpleBitCrossover<URBG>(6, urbg));
    pipeline.emplace_back(new WeightedTermCrossover<URBG>(6, urbg));
    // pipeline.emplace_back(new WeightedBitCrossover<URBG>(6, urbg));

    // Mutation strategy
    // pipeline.emplace_back(new NormalDistRandomMutation<URBG>(0.1, 0.02, urbg));
    pipeline.emplace_back(new BimodalNormalDistRandomMutation<URBG>(0.1, 0.02, 0.3, 0.05, 0.8, urbg));
    // pipeline.emplace_back(new UniformDistRandomMutation<URBG>(0.5, urbg));

    // Recalculate the objective functions after crossover and mutation
    pipeline.emplace_back(new ReevaluatePopulation());

    const float dont_care_weight = 5.0;
    const float all_zero_prob = 1.0f / num_inputs;

    auto objective_func = objective_function.clone();
    return std::make_unique<MemeticAlgorithm>(
            // Objective Function
            std::move(objective_func),

            // Solution Generator
            // std::make_unique<LocalSearchGenerator<URBG>>(num_inputs,
            //                                              num_outputs,
            //                                              max_terms,
            //                                              dont_care_weight,
            //                                              all_zero_prob,
            //                                              urbg,
            //                                              LocalSearchMode::BEST_IMPROVEMENT),

            local_search ? std::make_unique<LocalSearchGenerator<URBG>>(num_inputs,
                                                                        num_outputs,
                                                                        max_terms,
                                                                        dont_care_weight,
                                                                        all_zero_prob,
                                                                        urbg,
                                                                        LocalSearchMode::FIRST_IMPROVEMENT)
                         : std::make_unique<RandomGenerator<URBG>>(num_inputs,
                                                                   num_outputs,
                                                                   max_terms,
                                                                   dont_care_weight,
                                                                   all_zero_prob,
                                                                   urbg),

            std::make_unique<Pipeline>(std::move(pipeline)),
            // Reconstruction Strategies
            std::make_unique<PurePlusOperator>(),
            // std::make_unique<PureCommaOperator>(),

            // Convergence Strategies
            std::make_unique<FullConvergence>(),
            // std::make_unique<PercentConvergence>(0.99),
            // std::make_unique<AlwaysFalseConvergence>(),

            0.1);
}

template<class URBG, bool local_search>
std::unique_ptr<MemeticAlgorithm> createMultiThreadedAlgorithm( uint64_t num_inputs,
                                                                uint64_t num_outputs,
                                                                uint64_t max_terms,
                                                                ObjectiveFunction& objective_function,
                                                                URBG& urbg )
{
    std::vector<std::unique_ptr<PipelineStage>> pipeline;
    // Selection strategy
    // pipeline.emplace_back(new RankSelection<URBG>(local_search ? 0.25 : 0.6, urbg));
    pipeline.emplace_back(new TournamentSelection<URBG>(local_search ? 0.25 : 0.6, 5, urbg));
    // pipeline.emplace_back(new ElitistSelection(0.6, urbg));
    // pipeline.emplace_back(new PassthroughSelection());

    if ( local_search ) {
        // Local Search pipeline stage - done here cause the population is the smallest
        pipeline.emplace_back(new LocalSearchPopulation(LocalSearchMode::PARALLEL,
                                                        ThreadPool::getThreadPool().getNumThreads()));
    }

    // Crossover strategy
    // pipeline.emplace_back(new ParallelSimpleTermCrossover<URBG>(6, urbg));
    // pipeline.emplace_back(new ParallelSimpleBitCrossover<URBG>(6, urbg));
    pipeline.emplace_back(new ParallelWeightedTermCrossover<URBG>(6, urbg));
    // pipeline.emplace_back(new ParallelWeightedBitCrossover<URBG>(6, urbg));

    // Mutation strategy
    // pipeline.emplace_back(new ParallelNormalDistRandomMutation<URBG>(0.1, 0.02, urbg));
    pipeline.emplace_back(new ParallelBimodalNormalDistRandomMutation<URBG>(0.1, 0.02, 0.3, 0.05, 0.8, urbg));
    // pipeline.emplace_back(new ParallelUniformDistRandomMutation<URBG>(0.5, urbg));

    // Recalculate the objective functions after crossover and mutation
    pipeline.emplace_back(new ParallelReevaluatePopulation());

    const float dont_care_weight = 5.0;
    const float all_zero_prob = 1.0f / num_inputs;

    return std::make_unique<MemeticAlgorithm>(objective_function.clone(),
            // Solution Generator
                                              local_search ? std::make_unique<LocalSearchGenerator<URBG>>(num_inputs,
                                                                                                          num_outputs,
                                                                                                          max_terms,
                                                                                                          dont_care_weight,
                                                                                                          all_zero_prob,
                                                                                                          urbg,
                                                                                                          LocalSearchMode::PARALLEL)
                                                           : std::make_unique<RandomGenerator<URBG>>(num_inputs,
                                                                                                     num_outputs,
                                                                                                     max_terms,
                                                                                                     dont_care_weight,
                                                                                                     all_zero_prob,
                                                                                                     urbg),

                                              std::make_unique<Pipeline>(std::move(pipeline)),
            // Reconstruction Strategies
                                              std::make_unique<PurePlusOperator>(),
            // std::make_unique<PureCommaOperator>(),

            // Convergence Strategies
                                              std::make_unique<FullConvergence>(),
            // std::make_unique<PercentConvergence>(0.99),
            // std::make_unique<AlwaysFalseConvergence>(),
                                              0.1);
}

uint64_t max_objective_calls = 1e6;
uint64_t population_size = 1024;

void printHeader()
{
    std::cout << "Algorithm Name,Best Score,Num Objective Calls,Num Hash Calls,Total Time" << std::endl;
}

void printRes( const std::string& name,
               double min_score,
               uint64_t duration )
{
    if ( !csv ) {
        std::cout << name << " true score: " << min_score << std::endl;
        std::cout << "Objective calls: " << ObjectiveFunction::getNumCalls() << std::endl;
        std::cout << "Hash calls: " << HashFunction::getNumCalls() << std::endl;
        std::cout << "Time taken: " << duration << "ms" << std::endl;
        std::cout << std::endl;
    } else {
        std::cout << name << "," << min_score << "," << ObjectiveFunction::getNumCalls() << ","
                  << HashFunction::getNumCalls() << "," << duration << std::endl;
    }
}


void runAlgorithm( MemeticAlgorithm& algorithm,
                   ObjectiveFunction& true_objective_function,
                   const char* name )
{
    ObjectiveFunction::resetCalls();
    HashFunction::resetCalls();
    if ( !csv )
        std::cout << "Running " << name << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    HashFunction best = algorithm.run(population_size,
                                      [&]( uint64_t iter,
                                           Population& population )
                                      {
                                          if ( population.best().second == 0 ) {
                                              if ( true_objective_function(population.best().first) == 0 )
                                                  return true;
                                              else
                                                  population.reevaluate(algorithm.getObjectiveFunction());
                                          }
                                          return ObjectiveFunction::getNumCalls() >= max_objective_calls;
                                      });
    // Include this in the total calls/timing since parallel does
    double min_score = true_objective_function.normalize(true_objective_function(best));
    auto end = std::chrono::high_resolution_clock::now();
    printRes(name, min_score, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

void parallelRunAlgorithm( std::function<std::unique_ptr<MemeticAlgorithm>( uint64_t )> builder,
                           ObjectiveFunction& true_objective_function,
                           const char* name,
                           uint64_t num_threads )
{
    ThreadPool pool(num_threads);
    ObjectiveFunction::resetCalls();
    HashFunction::resetCalls();

    if ( !csv )
        std::cout << "Running " << name << " on " << num_threads << " threads" << std::endl;

    std::mutex objective_guard;
    std::vector<double> results(num_threads);
    auto start = std::chrono::high_resolution_clock::now();
    std::atomic<bool> done{false};
    pool.run([&]( uint64_t tid )
             {
                 auto algorithm = builder(tid);
                 HashFunction best = algorithm->run(population_size,
                                                    [&]( uint64_t iter,
                                                         Population& population )
                                                    {
                                                        if ( done )
                                                            return true;
                                                        if ( population.best().second == 0 ) {
                                                            uint64_t true_score;
                                                            {
                                                                std::lock_guard<std::mutex> l(objective_guard);
                                                                true_score = true_objective_function(population.best().first);
                                                            }
                                                            if ( true_score == 0 ) {
                                                                done = true;
                                                                return true;
                                                            } else {
                                                                population.reevaluate(algorithm->getObjectiveFunction());
                                                            }
                                                        }
                                                        return ObjectiveFunction::getNumCalls() >= max_objective_calls;
                                                    });
                 std::lock_guard<std::mutex> l(objective_guard);
                 results[tid] = true_objective_function.normalize(true_objective_function(best));
             }, num_threads);

    double min_score = *std::min(results.begin(), results.end());
    auto end = std::chrono::high_resolution_clock::now();
    printRes("Many " + std::string(name),
             min_score,
             std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

template<class URBG, bool local_search = false>
void evaluateHash( ObjectiveFunction& full_objective_function,
                   ObjectiveFunction& partial_objective_function,
                   URBG& urbg,
                   uint64_t num_inputs,
                   uint64_t num_outputs,
                   uint64_t max_terms,
                   uint64_t thread_pool_size )
{
    std::vector<std::mt19937_64> twisters;
    for ( uint64_t i = 0; i < thread_pool_size; i++ ) {
        twisters.emplace_back(urbg());
    }

    const float dont_care_weight = 5.0;
    const float all_zero_prob = 1.0f / num_inputs;

    {
        std::unique_ptr<MemeticAlgorithm> sequential_memetic_algorithm = createSingleThreadedAlgorithm<URBG, local_search>(
                num_inputs,
                num_outputs,
                max_terms,
                partial_objective_function,
                urbg);
        runAlgorithm(*sequential_memetic_algorithm, full_objective_function, "Sequential Memetic");
        sequential_memetic_algorithm.reset(nullptr);
    }

    parallelRunAlgorithm([&]( uint64_t tid )
                         {
                             return createSingleThreadedAlgorithm<URBG, local_search>(num_inputs,
                                                                                      num_outputs,
                                                                                      max_terms,
                                                                                      partial_objective_function,
                                                                                      twisters[tid]);
                         }, full_objective_function, "Sequential Memetic", thread_pool_size);

    {
        auto parallel_memetic_algorithm = createMultiThreadedAlgorithm<URBG, local_search>(num_inputs,
                                                                                           num_outputs,
                                                                                           max_terms,
                                                                                           partial_objective_function,
                                                                                           urbg);
        runAlgorithm(*parallel_memetic_algorithm, partial_objective_function, "Parallel Memetic");
    }


    {
        RandomWalk walk(partial_objective_function.clone(),
                        std::make_unique<RandomGenerator<std::mt19937_64>>(num_inputs,
                                                                           num_outputs,
                                                                           max_terms,
                                                                           dont_care_weight,
                                                                           all_zero_prob,
                                                                           urbg),
                        urbg);
        runAlgorithm(walk, partial_objective_function, "Random Walk");
    }

    parallelRunAlgorithm([&]( uint64_t tid )
                         {
                             std::mt19937_64& sub_twister = twisters[tid];
                             return std::make_unique<RandomWalk>(partial_objective_function.clone(),
                                                                 std::make_unique<RandomGenerator<std::mt19937_64>>(
                                                                         num_inputs,
                                                                         num_outputs,
                                                                         max_terms,
                                                                         dont_care_weight,
                                                                         all_zero_prob,
                                                                         sub_twister),
                                                                 sub_twister);
                         }, full_objective_function, "Random Walk", thread_pool_size);

    {
        RandomWalk parallel_walk(partial_objective_function.clone(),
                                 std::make_unique<RandomGenerator<std::mt19937_64>>(num_inputs,
                                                                                    num_outputs,
                                                                                    max_terms,
                                                                                    dont_care_weight,
                                                                                    all_zero_prob,
                                                                                    urbg),
                                 urbg,
                                 true);
        runAlgorithm(parallel_walk, full_objective_function, "Parallel Random Walk");
    }
}