#include <cstring>
#include <math.h>
#include "Evaluate.h"

int main( int argc,
          char** argv )
{
    if ( argc < 4 || argc > 5 ) {
        printf("Usage: %s <num inputs> <num outputs> <max terms> <hash seed>\n", argv[0]);
        return -1;
    }
    uint64_t seed = std::random_device()();
    if ( argc >= 5 )
        seed = atoi(argv[4]);
    std::mt19937_64 twister(seed);
    std::mt19937_64 twister2(std::random_device{}());

    uint64_t num_inputs = atoi(argv[1]);
    uint64_t num_outputs = atoi(argv[2]);
    uint64_t max_terms = atoi(argv[3]);
    const float dont_care_weight = 11.4;
    const float all_zero_prob = 0.0;//1.0f / num_inputs;

    RandomGenerator hash_gen(num_inputs, num_outputs, max_terms, dont_care_weight, all_zero_prob, twister);
    HashFunction h;
    float score = 0.0;
    // Loop while the hash function is heavily biased
    do {
        h = hash_gen();
        std::uniform_int_distribution<uint64_t> input_dist(0, ~0ull >> (64 -num_inputs));
        score = 0;
        uint64_t num_samples = 65536u;
        for ( uint64_t i = 0; i < num_samples; i++ ) {
            uint64_t val = input_dist(twister);
            uint64_t hash = h(val);
            uint64_t ones = __builtin_popcountll(hash);
            score += ones;
        }
        // std::cerr << score << "/" << num_samples * num_outputs << std::endl;
        score /= num_samples * num_outputs;
        // std::cerr << "Score " << score << std::endl;
    } while ( abs(score - 0.5) > 0.1 );

     uint64_t num_samples_pow = std::min(num_inputs, 20ul);
     bool rand = num_samples_pow < num_inputs;
     std::uniform_int_distribution<uint64_t> d(0, (~0ull) >> (64-num_inputs));
     std::cout << num_inputs << "," << num_outputs << "," << (1ull << num_samples_pow) << '\n';
     for(uint64_t i = 0; i < 1ull <<num_samples_pow; i++)
     {
         uint64_t val = i;
         if(rand)
             val = d(twister2);
         std::cout << val << "," << h(val) << '\n';
     }

//    try {
//        FILE* new_stdin = std::freopen(nullptr, "rb", stdin);
//        if ( !new_stdin || std::ferror(stdin) )
//            throw std::runtime_error(std::strerror(errno));
//
//        uint64_t buf = 0;
//        auto len = std::fread(&buf, 1, sizeof(uint64_t), stdin);
//        if(len * 8 != num_inputs)
//            throw std::runtime_error("Bad input data");
//        std::cout << std::hex << h(buf) << std::endl;
//    }
//    catch ( std::exception const& e ) {
//        std::cerr << e.what() << '\n';
//        return -1;
//    }
    return 0;
}