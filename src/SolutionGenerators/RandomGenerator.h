/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "../HashFunction.h"
#include "SolutionGenerator.h"
#include <cstdint>
#include <random>

template<class URBG>
class RandomGenerator : public SolutionGenerator
{
public:
    RandomGenerator( uint64_t input_bits,
                     uint64_t output_bits,
                     uint64_t max_depth,
                     float dont_care_weight,
                     float all_zero_prob,
                     URBG& urbg )
            : input_bits(input_bits)
            , output_bits(output_bits)
            , max_depth(max_depth)
            , dont_care_weight(dont_care_weight)
            , all_zero_prob(all_zero_prob)
            , urbg(urbg)
    {
    }

    HashFunction operator()()
    {
        const float keep_weight = 1.0;
        const float invert_weight = 1.0;
        std::uniform_real_distribution<float> choice_dist(0, keep_weight + invert_weight + dont_care_weight);
        std::uniform_real_distribution<float> uniform01;
        HashFunction h(input_bits, output_bits, max_depth);
        auto& plas = h.getPLAs();
        for ( auto& pla : plas ) {
            for ( auto& term : pla ) {
                if ( uniform01(urbg) < all_zero_prob )
                    continue;
                uint64_t num_set = input_bits;
                for ( uint64_t i = 0; i < input_bits; i++ ) {
                    float choice = choice_dist(urbg);
                    if ( choice < keep_weight ) {
                        term.set(i, Term::BitValue::KEEP);
                    } else if ( choice < keep_weight + invert_weight ) {
                        term.set(i, Term::BitValue::INVERT);
                    } else {
                        term.set(i, Term::BitValue::DONT_CARE);
                        num_set--;
                    }
                }
                // Set negated with probability proportional to the number of set bits
                // term.setNegation(uniform01(urbg) < (0.5 / (num_set * num_set + 1.0)));
            }
        }

        return h;
    }

    HashFunction operator()( const MemeticAlgorithm& ) override
    {
        return operator()();
    }

protected:
    const uint64_t input_bits;
    const uint64_t output_bits;
    const uint64_t max_depth;
    const float dont_care_weight;
    const float all_zero_prob;
    URBG& urbg;
};
