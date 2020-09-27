#include "src/HashFunction.h"
#include "src/LocalSearch.h"
#include "src/MemeticAlgorithm.h"
#include "src/ObjectiveFunction.h"
#include "src/ReconstructionStrategies/CommaOperator.h"
#include "src/ReconstructionStrategies/PlusOperator.h"
#include "src/SolutionGenerators/LocalSearchGenerator.h"
#include "src/SolutionGenerators/RandomGenerator.h"
#include "src/RecombinationOperators/SimpleCrossover.h"
#include "src/MutationStrategies/RandomMutation.h"
#include "src/ConvergenceCriteria/AlwaysFalseConvergence.h"
#include "src/ConvergenceCriteria/FullConvergence.h"
#include "src/SelectionStrategies/PassthroughSelection.h"
#include "src/SelectionStrategies/ElitistSelection.h"
#include "src/SelectionStrategies/TournamentSelection.h"
#include "src/LocalSearchPopulation.h"

#include <iostream>


int main()
{
    uint64_t seed = std::random_device()();
    std::cout << "Seed: " << seed << std::endl;
    std::mt19937_64 twister(seed);

    uint64_t num_inputs = 8;
    uint64_t num_outputs = 1;
    uint64_t max_terms = 8;
    const float dont_care_weight = 3.0;
    const float all_zero_prob = 1.0f / num_inputs;

    RandomGenerator hash_gen(num_inputs, num_outputs, max_terms, dont_care_weight, all_zero_prob, twister);
    HashFunction h ;
    float score = 0;
    // Loop while the hash function is heavily biased
    while (abs(score-0.5) > 0.1) {
        h = hash_gen();
        std::uniform_int_distribution<uint64_t> input_dist(0, (1u << num_inputs) - 1);
        score = 0;
        uint64_t num_samples = 1u << num_inputs;
        for ( uint64_t i = 0; i < num_samples; i++ ) {
            uint64_t val = input_dist(twister);
            uint64_t hash = h(val);
            uint64_t ones = __builtin_popcountll(hash);
            score += ones;
        }
        score /= num_samples * num_outputs;
        std::cout << "Balance score (0 to 1): " << score << std::endl;
    }

    std::vector<std::unique_ptr<PipelineStage>> pipeline;
    // Selection strategy
    // pipeline.emplace_back(new TournamentSelection<std::mt19937_64>( 0.6, 5, twister ));
    // pipeline.emplace_back(new ElitistSelection( 0.6, twister ));
    pipeline.emplace_back(new PassthroughSelection());

    // Local Search pipeline stage
    // pipeline.emplace_back(new LocalSearchPopulation(LocalSearchMode::FIRST_IMPROVEMENT));
    pipeline.emplace_back(new LocalSearchPopulation(LocalSearchMode::BEST_IMPROVEMENT));

    // Crossover strategy
    pipeline.emplace_back(new SimpleTermCrossover<std::mt19937_64>(6, twister));

    // Mutation strategy
    // pipeline.emplace_back(new NormalDistRandomMutation<std::mt19937_64>(0.1, 0.02, twister));
    pipeline.emplace_back(new UniformDistRandomMutation<std::mt19937_64>(0.2, twister));

    MemeticAlgorithm mimetic_algorithm(std::make_unique<CachedObjectiveFunction>(h,
                                                                                 std::min(1u << num_inputs, 65536u),
                                                                                 twister),
                                       std::make_unique<RandomGenerator<std::mt19937_64>>(num_inputs,
                                                                                          num_outputs,
                                                                                          max_terms,
                                                                                          dont_care_weight,
                                                                                          all_zero_prob,
                                                                                          twister),
                                       std::make_unique<Pipeline>(pipeline),
                                       std::make_unique<PurePlusOperator>(),
                                       std::make_unique<FullConvergence>(),
                                       0.3);

    HashFunction best = mimetic_algorithm.run(100,
                                              [&]( uint64_t iter,
                                                   uint64_t best_score )
                                              {
                                                  return iter >= 10000 || best_score == 0;
                                              });

    for ( uint64_t i = 0; i < (1u << num_inputs); i++ ) {
        if ( best(i) != h(i) )
            std::cout << i << " | " << best(i) << " != " << h(i) << std::endl;
    }

    UncachedObjectiveFunction objective_function(h, 1u << num_inputs, twister);
    std::cout << "True score: " << objective_function(best) << std::endl;
}
