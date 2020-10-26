/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "HashFunction.h"
#include "ObjectiveFunction.h"
#include "ParallelLocalSearch.h"
#include <memory>

enum class LocalSearchMode
{
    BEST_IMPROVEMENT,
    FIRST_IMPROVEMENT,
    PARALLEL
};

class LocalSearch
{
public:
    LocalSearch( LocalSearchMode mode,
                 uint64_t num_threads = 1);

    std::pair<HashFunction, uint64_t> operator()( const HashFunction& in ) const;

    void setObjectiveFunction(ObjectiveFunction& objective_function);
private:
    uint64_t improve( HashFunction& function,
                      uint64_t current_val ) const;

    const LocalSearchMode search_mode;
    std::unique_ptr<ParallelLocalSearch> parallel_search;
    ObjectiveFunction* objective_function;
};
