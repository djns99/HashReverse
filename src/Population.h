/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "HashFunction.h"
#include "ObjectiveFunction.h"
#include <utility>
#include <vector>

class Population
{
public:
    Population() = default;

    Population( const Population& ) = default;

    Population( Population&& ) = default;

    Population& operator=( const Population& other ) = default;

    Population& operator=( Population&& ) = default;

    explicit Population( uint64_t reserve );

    void add( HashFunction function,
              uint64_t score );

    void remove( HashFunction& function );

    [[nodiscard]] uint64_t size() const;

    void resize( uint64_t size );

    void reheap();

    void sort();

    [[nodiscard]] const std::vector<std::pair<HashFunction, uint64_t>>& getMembers() const;

    [[nodiscard]] std::vector<std::pair<HashFunction, uint64_t>>& getMembers();

    [[nodiscard]] const std::pair<HashFunction, uint64_t>& best() const;

    std::pair<HashFunction, uint64_t> popHead();

    void reevaluate( ObjectiveFunction& objective_function );

    [[nodiscard]] bool isHeap() const;
private:
    std::vector<std::pair<HashFunction, uint64_t>> members;

    struct CompareGreater
    {
        bool operator()( const std::pair<HashFunction, uint64_t>& left,
                         const std::pair<HashFunction, uint64_t>& right ) const
        {
            return left.second > right.second;
        }
    };

    CompareGreater heap_comparator{};
};
