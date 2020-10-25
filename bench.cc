/**
 * Daniel Stokes 1331334
 */
#define CSV true
#include "Evaluate.h"

int main(int argc, char** argv)
{
    if(argc < 6 || argc > 10)
    {
        printf("Usage: %s <test file> <validation file> <num threads> <max terms> <partial samples> [<objective evaluations> <population size> <seed> <local search>]\n", argv[0]);
        return -1;
    }

    uint64_t threads = atoi(argv[3]);
    auto& tp = ThreadPool::getThreadPool(threads);
    uint64_t thread_pool_size = tp.getNumThreads();

    uint64_t seed = std::random_device()();
    if(argc >= 9)
        seed = atoi(argv[8]);
    else
        std::cerr << "Seed: " << seed << std::endl;
    std::mt19937_64 twister(seed);

    FileReaderObjectiveFunction partial(argv[1], twister, atoi(argv[5]));
    FileReaderObjectiveFunction full(argv[2], twister);
    uint64_t max_terms = atoi(argv[4]);

    if(argc >= 7)
        max_objective_calls = atoi(argv[6]);
    if(argc >= 8)
        population_size = atoi(argv[7]);

    printHeader();
    if(argc >= 10 && argv[9][0] == '1')
        evaluateHash<std::mt19937_64, true>(full, partial, twister, full.getInputBits(), full.getOutputBits(), max_terms, thread_pool_size);
    else
        evaluateHash(full, partial, twister, full.getInputBits(), full.getOutputBits(), max_terms, thread_pool_size);
}

