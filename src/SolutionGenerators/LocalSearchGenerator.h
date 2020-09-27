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
                          uint64_t dont_care_weight,
                          URBG& urbg,
                          LocalSearchMode mode,
                          ObjectiveFunction& objective_function )
            : RandomGenerator<URBG>(input_bits, output_bits, max_depth, dont_care_weight, urbg)
            , search(mode, objective_function)
    {
    }

    HashFunction operator()() override
    {
        return search(RandomGenerator<URBG>::operator()());
    }

private:
    LocalSearch search;
};
