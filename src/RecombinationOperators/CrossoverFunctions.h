#pragma once
#include "../HashFunction.h"
#include <random>
class TermCrossover
{
public:
    template<class URBG>
    static HashFunction crossover( const HashFunction& h1,
                            const HashFunction& h2,
                                   URBG& urbg)
    {
        std::uniform_int_distribution<int> dist01;
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
                    bool use_first = dist01(urbg);
                    term_out = use_first ? term1 : term2;
                } else {
                    // Combine valid terms together
                    for ( uint64_t var = 0; var < h1.getInputBits(); var++ ) {
                        bool use_first = dist01(urbg);
                        term_out.set(var, use_first ? term1.get(var) : term2.get(var));
                    }
                }
            }
        }
        return output;
    }
};

class BitCrossover
{
public:
    template<class URBG>
    static HashFunction crossover( const HashFunction& h1,
                            const HashFunction& h2,
                                   URBG& urbg)
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
                bool use_first = dist(urbg);
                term_out = use_first ? term1 : term2;
            }
        }
        return output;
    }
};