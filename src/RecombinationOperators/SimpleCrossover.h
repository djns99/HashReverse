#pragma once

#include "../PipelineStage.h"
#include <random>

template<class URBG>
class SimpleCrossover : public PipelineStage
{
public:
    SimpleCrossover( double new_pop_ratio,
                     URBG& urbg )
            : new_pop_ratio(new_pop_ratio)
            , urbg(urbg)
    {
    }

    void operator()( MemeticAlgorithm& algorithm,
                     Population& old_pop ) override
    {
        const uint64_t old_size = old_pop.size();
        assert(old_size != 0);
        const uint64_t new_size = algorithm.getPopulationSize() * new_pop_ratio;
        auto& old_functions = old_pop.getMembers();
        ObjectiveFunction& objective_function = algorithm.getObjectiveFunction();
        Population new_pop(new_size);
        std::uniform_int_distribution<uint64_t> dist(0, old_size - 1);
        while ( new_pop.size() < new_size ) {
            if ( old_size > 1 ) {
                const uint64_t first_idx = dist(urbg);
                const uint64_t second_idx = dist(urbg);
                if ( first_idx == second_idx ) {
                    continue;
                }
                auto new_func = crossover(old_functions[first_idx].first, old_functions[second_idx].first);
                uint64_t score = objective_function(new_func);
                new_pop.add(std::move(new_func), score);
            } else {
                new_pop.add(old_functions[0].first, old_functions[0].second);
            }
        }
        old_pop = std::move(new_pop);
    }

protected:
    virtual HashFunction crossover( const HashFunction& h1,
                                    const HashFunction& h2 ) = 0;

    double new_pop_ratio;
    URBG& urbg;
};

template<class URBG>
class SimpleBitCrossover : public SimpleCrossover<URBG>
{
public:
    SimpleBitCrossover( double new_pop_ratio,
                        URBG& urbg )
            : SimpleCrossover<URBG>(new_pop_ratio, urbg)
    {
    }

protected:
    HashFunction crossover( const HashFunction& h1,
                            const HashFunction& h2 ) override
    {
        std::uniform_int_distribution<bool> dist01;
        HashFunction output(h1.getInputBits(), h1.getOutputBits(), h1.getMaxTerms());
        const auto& plas1 = h1.getPLAs();
        const auto& plas2 = h2.getPLAs();
        auto& plas_out = output.getPLAs();
        for ( uint64_t pla = 0; pla < h1.getOutputBits(); pla++ ) {
            const auto& pla1 = plas1[pla];
            const auto& pla2 = plas2[pla];
            auto& pla_out = plas_out[pla];
            for ( uint64_t term = 0; term < h1.getMaxTerms(); term++ ) {
                const Term& term1 = pla1[term];
                const Term& term2 = pla2[term];
                Term& term_out = pla_out[term];
                if ( !term1.isInit() || !term2.isInit() || term1.isNegated() != term2.isNegated()) {
                    // Select between an actual term or a completely false one
                    bool use_first = dist01(SimpleCrossover<URBG>::urbg);
                    term_out = use_first ? term1 : term2;
                } else {
                    // Combine valid terms together
                    for ( uint64_t var = 0; var < h1.getInputBits(); var++ ) {
                        bool use_first = dist01(SimpleCrossover<URBG>::urbg);
                        term_out.set(var, use_first ? term1.get(var) : term2.get(var));
                    }
                }
            }
        }
        return output;
    }
};

template<class URBG>
class SimpleTermCrossover : public SimpleCrossover<URBG>
{
public:
    SimpleTermCrossover( double new_pop_ratio,
                         URBG& urbg )
            : SimpleCrossover<URBG>(new_pop_ratio, urbg)
    {
    }

protected:
    HashFunction crossover( const HashFunction& h1,
                            const HashFunction& h2 ) override
    {
        std::uniform_int_distribution<int> dist(0, 1);
        HashFunction output(h1.getInputBits(), h1.getOutputBits(), h1.getMaxTerms());
        const auto& plas1 = h1.getPLAs();
        const auto& plas2 = h2.getPLAs();
        auto& plas_out = output.getPLAs();
        for ( uint64_t pla = 0; pla < h1.getOutputBits(); pla++ ) {
            const auto& pla1 = plas1[pla];
            const auto& pla2 = plas2[pla];
            auto& pla_out = plas_out[pla];
            for ( uint64_t term = 0; term < h1.getMaxTerms(); term++ ) {
                const Term& term1 = pla1[term];
                const Term& term2 = pla2[term];
                Term& term_out = pla_out[term];

                // Select one of the parent terms
                bool use_first = dist(this->urbg);
                term_out = use_first ? term1 : term2;
            }
        }
        return output;
    }
};