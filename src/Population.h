#pragma once
#include "HashFunction.h"
#include <vector>
#include <utility>

class Population
{
public:
    Population() = default;
    Population(const Population&) = default;
    Population(Population&&) = default;
    Population& operator=(const Population&) = default;
    Population& operator=(Population&&) = default;
    explicit Population(uint64_t reserve);
    void add(HashFunction function, uint64_t score);
    uint64_t remove(HashFunction& function);
    [[nodiscard]] uint64_t size() const;

    [[nodiscard]] const std::vector<std::pair<HashFunction, uint64_t>>& getMembers() const;
    [[nodiscard]] std::pair<HashFunction, uint64_t> best() const;

    std::pair<HashFunction, uint64_t> popHead();
private:
    std::vector<std::pair<HashFunction, uint64_t>> members;
    struct Compare
    {
        bool operator()(const std::pair<HashFunction, uint64_t>& left, const std::pair<HashFunction, uint64_t>& right) const
        {
            return left.second < right.second;
        }
    };
    Compare comparator;
};
