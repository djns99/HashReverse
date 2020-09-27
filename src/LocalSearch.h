#pragma once

#include "HashFunction.h"
#include "ObjectiveFunction.h"
#include <memory>

enum class LocalSearchMode
{
    BEST_IMPROVEMENT,
    FIRST_IMPROVEMENT
};

class LocalSearch
{
public:
    LocalSearch( LocalSearchMode mode,
                 ObjectiveFunction& function );

    std::pair<HashFunction, uint64_t> operator()( const HashFunction& in ) const;

private:
    uint64_t improve( HashFunction& function,
                      uint64_t current_val ) const;

    const LocalSearchMode search_mode;
    ObjectiveFunction& objective_function;
};
