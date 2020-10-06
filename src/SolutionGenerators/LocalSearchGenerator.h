#pragma once

#include "../LocalSearch.h"
#include "../ObjectiveFunction.h"
#include "RandomGenerator.h"

template<class URBG>
class LocalSearchGenerator : public RandomGenerator<URBG>
{
public:
    LocalSearchGenerator( uint64_t input_bits,
                          uint64_t output_bits,
                          uint64_t max_depth,
                          float dont_care_weight,
                          float all_zero_prob,
                          URBG& urbg,
                          LocalSearchMode mode )
            : RandomGenerator<URBG>(input_bits, output_bits, max_depth, dont_care_weight, all_zero_prob, urbg)
            , search_mode(mode)
    {
    }

    HashFunction operator()( const MemeticAlgorithm& memetic_algorithm ) override
    {
        LocalSearch search(search_mode, memetic_algorithm.getObjectiveFunction());
        return search(RandomGenerator<URBG>::operator()()).first;
    }

private:
    LocalSearchMode search_mode;
};
