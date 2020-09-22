#include "Population.h"
#include <algorithm>

void Population::add(HashFunction function, uint64_t score)
{
    members.emplace_back(std::move(function), score);
    std::push_heap(members.begin(), members.end(), comparator);
}

uint64_t Population::remove(HashFunction& function)
{
    for(auto& f : members)
    {
        if(&f.first == &function)
        {
            std::swap(f, members.back());
            std::make_heap(members.begin(), members.end(), comparator);
        }
    }
}
std::pair<HashFunction, uint64_t> Population::best() const
{
    return members[0];
}
const std::vector<std::pair<HashFunction, uint64_t>>& Population::getMembers() const
{
    return members;
}
std::pair<HashFunction, uint64_t> Population::popHead()
{
    std::pop_heap(members.begin(), members.end(), comparator);
    auto head = members.back();
    members.pop_back();
    return head;
}
Population::Population(uint64_t reserve)
{
    members.reserve(reserve);
}
uint64_t Population::size() const
{
    return members.size();
}
