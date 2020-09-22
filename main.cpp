#include "src/HashFunction.h"
#include "src/LocalSearch.h"
#include "src/ObjectiveFunction.h"
#include <iostream>

template<class URBG>
HashFunction genRandomFunction(uint64_t num_inputs, uint64_t num_outputs, uint64_t max_terms, URBG&& urbg)
{
    std::uniform_int_distribution<uint64_t> dist(0, 3);
    HashFunction h(num_inputs, num_outputs, max_terms);
    auto& plas = h.getPLAs();
    for(auto& pla : plas)
    {
        for(auto& term : pla)
        {
            for(uint64_t i = 0; i < num_inputs; i++)
            {
                uint64_t val = dist(urbg);
                if(val > 2)
                    term.set(i, Term::BitValue::DONT_CARE);
                else
                    term.set(i, (Term::BitValue)(val + 1));
            }
        }
    }

    return h;
}

int main()
{
    uint64_t seed = std::random_device()();
    std::cout << "Seed: " << seed << std::endl;
    std::mt19937_64 twister(seed);

    uint64_t num_inputs = 8;
    uint64_t num_ouputs = 8;
    uint64_t max_terms = 8;

    HashFunction h = genRandomFunction(num_inputs, num_ouputs, max_terms, twister);
    for(uint64_t i = 0; i < (1u << num_inputs); i++)
    {
        std::cout << i << " | " << h(i) << std::endl;
    }

    UncachedObjectiveFunction f(h, 128, twister);
    std::cout << "Perfect score: " << f(h) << std::endl;

    HashFunction r1 = genRandomFunction(num_inputs, num_ouputs, max_terms, twister);
    std::cout << "Unoptimised score: " << f(r1) << std::endl;

    LocalSearch search1(LocalSearchMode::FIRST_IMPROVEMENT, f);
    for(uint64_t i = 0; i < 10; i++)
    {
        HashFunction improved = search1(r1);
        std::cout << "First improvement score: " << f(improved) << std::endl;
    }

    LocalSearch search2(LocalSearchMode::BEST_IMPROVEMENT, f);
    for(uint64_t i = 0; i < 10; i++)
    {
        HashFunction improved = search2(r1);
        std::cout << "Best improvement score: " << f(improved) << std::endl;
    }

    return 0;
}
