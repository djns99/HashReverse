#pragma once
#include "ObjectiveFunction.h"
#include "HashFunction.h"
#include <memory>

enum class LocalSearchMode
{
    BEST_IMPROVEMENT,
    FIRST_IMPROVEMENT
};

class LocalSearch
{
public:
    LocalSearch(LocalSearchMode mode, ObjectiveFunction& function);

    HashFunction operator()(const HashFunction& in) const;
private:
    uint64_t improve(HashFunction& function, uint64_t current_val) const;

    const LocalSearchMode search_mode;
    ObjectiveFunction& objective_function;
};

