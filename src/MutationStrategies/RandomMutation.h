#pragma once

#include "../PipelineStage.h"
#include <iostream>

template<class URBG>
class RandomMutation : public PipelineStage
{
protected:
    explicit RandomMutation( URBG& urbg,
                             float dont_care_weight = 1.0 )
            : urbg(urbg)
            , dont_care_weight(dont_care_weight)
    {
    }

    void mutate( const MemeticAlgorithm& mimetic_algorithm,
                 std::pair<HashFunction, uint64_t>& hash_function,
                 uint64_t num_mutations )
    {
        std::uniform_int_distribution<uint64_t> pla_dist(0, hash_function.first.getOutputBits() - 1);
        std::uniform_int_distribution<uint64_t> term_dist(0, hash_function.first.getMaxTerms() - 1);
        std::uniform_int_distribution<uint64_t> bit_dist(0, hash_function.first.getInputBits() - 1);
        const float keep_weight = 1.0;
        const float invert_weight = 1.0;
        const float base_weight = keep_weight + invert_weight + dont_care_weight;
        const float wipe_weight = 1.0 / pow(base_weight, hash_function.first.getInputBits() / 2.0);
        const float negate_weight = 1.0 / pow(base_weight, hash_function.first.getInputBits());
        std::uniform_real_distribution<float> choice_dist(0, base_weight + negate_weight + wipe_weight);
        for ( uint64_t i = 0; i < num_mutations; i++ ) {
            const uint64_t pla = pla_dist(urbg);
            const uint64_t term = term_dist(urbg);
            const uint64_t bit = bit_dist(urbg);
            const float mutation_type = choice_dist(urbg);

            if ( mutation_type >= base_weight + negate_weight ) {
                hash_function.first.getPLAs()[pla][term].clear();
            } else if ( mutation_type >= base_weight ) {
                hash_function.first.getPLAs()[pla][term].flipNegation();
            } else if ( mutation_type < keep_weight ) {
                hash_function.first.getPLAs()[pla][term].set(bit, Term::BitValue::KEEP);
            } else if ( mutation_type < keep_weight + invert_weight ) {
                hash_function.first.getPLAs()[pla][term].set(bit, Term::BitValue::INVERT);
            } else {
                hash_function.first.getPLAs()[pla][term].set(bit, Term::BitValue::DONT_CARE);
            }
        }

        hash_function.second = mimetic_algorithm.getObjectiveFunction()(hash_function.first);
    }

    URBG& urbg;
    const float dont_care_weight;
};

template<class URBG>
class UniformDistRandomMutation : public RandomMutation<URBG>
{
public:
    explicit UniformDistRandomMutation( float max_mutation_rate,
                                        URBG& urbg )
            : RandomMutation<URBG>(urbg)
            , max_mutation_rate(max_mutation_rate)
    {
    }

    void operator()( MemeticAlgorithm& mimetic_algorithm,
                     Population& pop ) override
    {
        std::uniform_real_distribution<float> dist(0, max_mutation_rate);
        const auto& best = pop.best();
        const uint64_t num_terms = best.first.getOutputBits() * best.first.getMaxTerms();
        for ( auto& member : pop.getMembers() ) {
            RandomMutation<URBG>::mutate(mimetic_algorithm, member, num_terms * dist(this->urbg));
        }
        pop.reheap();
    }

private:
    float max_mutation_rate;
};

template<class URBG>
class NormalDistRandomMutation : public RandomMutation<URBG>
{
public:
    explicit NormalDistRandomMutation( float mean_mutation_rate,
                                       float stddev,
                                       URBG& urbg )
            : RandomMutation<URBG>(urbg)
            , mean_mutation_rate(mean_mutation_rate)
            , stddev(stddev)
    {
    }

    void operator()( MemeticAlgorithm& mimetic_algorithm,
                     Population& pop ) override
    {
        std::normal_distribution<float> dist(mean_mutation_rate, stddev);
        const auto& best = pop.best();
        const uint64_t num_terms = best.first.getOutputBits() * best.first.getMaxTerms();
        for ( auto& member : pop.getMembers() ) {
            RandomMutation<URBG>::mutate(mimetic_algorithm, member, num_terms * dist(this->urbg));
        }
        pop.reheap();
    }

private:
    float mean_mutation_rate;
    float stddev;
};