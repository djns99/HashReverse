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

#include <iostream>

int main()
{
    uint64_t seed = std::random_device()();
    std::cout << "Seed: " << seed << std::endl;
    std::mt19937_64 twister(seed);

    uint64_t num_inputs = 32;
    uint64_t num_outputs = 1;
    uint64_t max_terms = 8;
    const float dont_care_weight = 10.0;
    const float all_zero_prob = 1.0f / num_inputs;

    RandomGenerator hash_gen(num_inputs, num_outputs, max_terms, dont_care_weight, all_zero_prob, twister);
    HashFunction h;
    float score = 0.0;
    // Loop while the hash function is heavily biased
    do {
        h = hash_gen();
        std::uniform_int_distribution<uint64_t> input_dist(0, (1ull << num_inputs) - 1);
        score = 0;
        uint64_t num_samples = 65536u;
        for ( uint64_t i = 0; i < num_samples; i++ ) {
            uint64_t val = input_dist(twister);
            uint64_t hash = h(val);
            uint64_t ones = __builtin_popcountll(hash);
            score += ones;
        }
        score /= num_samples * num_outputs;
        std::cout << "Balance score (0 to 1): " << score << std::endl;
    } while ( abs(score - 0.5) > 0.1 );

    std::vector<std::unique_ptr<PipelineStage>> pipeline;
    // Selection strategy
    pipeline.emplace_back(new RankSelection<std::mt19937_64>(0.6, twister));
    // pipeline.emplace_back(new TournamentSelection<std::mt19937_64>(0.6, 5, twister));
    // pipeline.emplace_back(new ElitistSelection( 0.6, twister ));
    // pipeline.emplace_back(new PassthroughSelection());

    // Local Search pipeline stage - done here cause the population is the smallest
    // pipeline.emplace_back(new LocalSearchPopulation(LocalSearchMode::FIRST_IMPROVEMENT));
    // pipeline.emplace_back(new LocalSearchPopulation(LocalSearchMode::BEST_IMPROVEMENT));

    // Crossover strategy
    // pipeline.emplace_back(new SimpleTermCrossover<std::mt19937_64>(6, twister));
    // pipeline.emplace_back(new SimpleBitCrossover<std::mt19937_64>(6, twister));
    pipeline.emplace_back(new WeightedTermCrossover<std::mt19937_64>(6, twister));
    // pipeline.emplace_back(new WeightedBitCrossover<std::mt19937_64>(6, twister));

    // Mutation strategy
    // pipeline.emplace_back(new NormalDistRandomMutation<std::mt19937_64>(0.1, 0.02, twister));
    pipeline.emplace_back(new BimodalNormalDistRandomMutation<std::mt19937_64>(0.1, 0.02, 0.3, 0.05, 0.8, twister));
    // pipeline.emplace_back(new UniformDistRandomMutation<std::mt19937_64>(0.5, twister));

    // Recalculate the objective functions after crossover and mutation
    pipeline.emplace_back(new ReevaluatePopulation());

    MemeticAlgorithm mimetic_algorithm(
            // Objective Function
            // std::make_unique<CachedObjectiveFunction>(h, std::min(1ull << num_inputs, 65536ull), twister),
            std::make_unique<UncachedObjectiveFunction<std::mt19937_64>>(h,
                                                                         std::min(1ull << num_inputs, 1024ull),
                                                                         twister),

            // Solution Generator
            std::make_unique<LocalSearchGenerator<std::mt19937_64>>(num_inputs,
                                                                    num_outputs,
                                                                    max_terms,
                                                                    dont_care_weight,
                                                                    all_zero_prob,
                                                                    twister,
                                                                    LocalSearchMode::FIRST_IMPROVEMENT),
            // std::make_unique<RandomGenerator<std::mt19937_64>>(num_inputs,
            //                                                    num_outputs,
            //                                                    max_terms,
            //                                                    dont_care_weight,
            //                                                    all_zero_prob,
            //                                                    twister),

            std::make_unique<Pipeline>(pipeline),
            // Reconstruction Strategies
            std::make_unique<PurePlusOperator>(),
            // std::make_unique<PureCommaOperator>(),

            // Convergence Strategies
            // std::make_unique<FullConvergence>(),
            std::make_unique<PercentConvergence>(0.99),
            // std::make_unique<AlwaysFalseConvergence>(),
            0.3);

    const uint64_t num_to_evaluate = 1ull << std::min(num_inputs, 20ul);
    UncachedObjectiveFunction objective_function(h, num_to_evaluate, twister);

    HashFunction best = mimetic_algorithm.run(100,
                                              [&]( uint64_t iter,
                                                   Population& population )
                                              {
                                                  if ( population.best().second == 0 ) {
                                                      if ( objective_function(population.best().first) == 0 )
                                                          return true;
                                                      else
                                                          population.reevaluate(mimetic_algorithm.getObjectiveFunction());
                                                  }
                                                  return iter >= 1000;
                                              });

    // for ( uint64_t i = 0; i < num_to_evaluate; i++ ) {
    //     if ( best(i) != h(i) )
    //         std::cout << i << " | " << best(i) << " != " << h(i) << std::endl;
    // }

    std::cout << "True score: " << objective_function.normalize(objective_function(best))
              << std::endl;
    std::cout << "Total calls: " << HashFunction::getNumCalls() << std::endl;
}
