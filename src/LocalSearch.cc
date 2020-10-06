#include "LocalSearch.h"
#include <iostream>

LocalSearch::LocalSearch( LocalSearchMode mode,
                          uint64_t num_threads )
        : search_mode(mode)
{
    if ( search_mode == LocalSearchMode::PARALLEL ) {
        parallel_search = std::make_unique<ParallelLocalSearch>(num_threads);
    } else {
        assert(num_threads == 1);
    }
}

std::pair<HashFunction, uint64_t> LocalSearch::operator()( const HashFunction& in ) const
{
    if ( search_mode == LocalSearchMode::PARALLEL ) {
        return (*parallel_search)(in);
    } else {
        uint64_t current_val = UINT64_MAX;
        uint64_t next_val = (*objective_function)(in);
        HashFunction f = in;
        while ( next_val < current_val ) {
            current_val = next_val;
            next_val = improve(f, current_val);
        }
        return {std::move(f), current_val};
    }
}

uint64_t LocalSearch::improve( HashFunction& function,
                               uint64_t current_val ) const
{
    auto& plas = function.getPLAs();
    uint64_t num_bits = function.getInputBits();
    Term best_term;
    Term* best_location = nullptr;
    for ( auto& pla : plas ) {
        for ( auto& term : pla ) {
            const Term old_term = term;
            for ( uint64_t term_bit = 0; term_bit < num_bits; term_bit++ ) {
                for ( uint64_t val = Term::KEEP; val <= Term::DONT_CARE; val++ ) {
                    if ( old_term.get(term_bit) != val ) {
                        term.set(term_bit, (Term::BitValue)val);
                        uint64_t score = (*objective_function)(function);
                        if ( score < current_val ) {
                            if ( search_mode == LocalSearchMode::FIRST_IMPROVEMENT ) {
                                return score;
                            }

                            best_term = term;
                            best_location = &term;
                            current_val = score;
                        }
                    }
                }

                if ( old_term.isInit() ) {
                    // Check the negated term
                    term.flipNegation();
                    uint64_t score = (*objective_function)(function);
                    if ( score < current_val ) {
                        if ( search_mode == LocalSearchMode::FIRST_IMPROVEMENT ) {
                            return score;
                        }

                        best_term = term;
                        best_location = &term;
                        current_val = score;
                    }

                    // Check the empty term
                    term.clear();
                    score = (*objective_function)(function);
                    if ( score < current_val ) {
                        if ( search_mode == LocalSearchMode::FIRST_IMPROVEMENT ) {
                            return score;
                        }

                        best_term = term;
                        best_location = &term;
                        current_val = score;
                    }
                }

                // Restore the original term
                term = old_term;
            }
        }
    }

    // Reapply the best improvement we found
    if ( best_location ) {
        assert(search_mode == LocalSearchMode::BEST_IMPROVEMENT);
        *best_location = best_term;
    }

    // Return
    return current_val;
}

void LocalSearch::setObjectiveFunction( ObjectiveFunction& objective_function )
{
    this->objective_function = &objective_function;
    if(parallel_search)
        parallel_search->setObjectiveFunction(objective_function);
}
