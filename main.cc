#include "Evaluate.h"

void random( int argc,
             char** argv )
{
    uint64_t thread_pool_size = std::thread::hardware_concurrency();
    if ( argc == 2 )
        thread_pool_size = atoi(argv[1]);
    ThreadPool::getThreadPool(thread_pool_size);

    uint64_t seed = std::random_device()();
    std::cout << "Seed: " << seed << std::endl;
    std::mt19937_64 twister(seed);

    uint64_t num_inputs = 16;
    uint64_t num_outputs = 1;
    uint64_t max_terms = 12;
    const float dont_care_weight = 5.0;
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

    UncachedObjectiveFunction partial(h, std::min(1ull << num_inputs, 1024ull), twister);
    UncachedObjectiveFunction full(h, std::min(1ull << num_inputs, 65536ull), twister);
    evaluateHash(full, partial, twister, num_inputs, num_outputs, max_terms, thread_pool_size);
}

void fileEvaluation( int argc,
                     char** argv )
{
    auto& tp = ThreadPool::getThreadPool(atoi(argv[1]));
    tp.resize(atoi(argv[1]));
    uint64_t thread_pool_size = tp.getNumThreads();

    uint64_t seed = std::random_device()();
    std::cerr << "Seed: " << seed << std::endl;
    std::mt19937_64 twister(seed);

    for ( int i = 2; i < argc; i++ ) {
        if ( !csv )
            std::cout << "Evaluating: " << argv[i] << std::endl;
        else
            std::cout << std::endl << argv[i] << std::endl;
        FileReaderObjectiveFunction full(argv[i], twister);
        FileReaderObjectiveFunction partial(argv[i], twister, 8192ull);
        uint64_t max_terms = full.getInputBits() * full.getInputBits();
        evaluateHash(full, partial, twister, full.getInputBits(), full.getOutputBits(), max_terms, thread_pool_size);
    }
}

int main( int argc,
          char** argv )
{
    if ( argc <= 2 )
        random(argc, argv);
    else if ( argc >= 3 )
        fileEvaluation(argc, argv);
    return 0;
}
