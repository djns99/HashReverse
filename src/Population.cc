#include "Population.h"
#include <algorithm>

void Population::add( HashFunction function,
                      uint64_t score )
{
    members.emplace_back(std::move(function), score);
    std::push_heap(members.begin(), members.end(), heap_comparator);
}

void Population::remove( HashFunction& function )
{
    for ( auto& f : members ) {
        if ( &f.first == &function ) {
            std::swap(f, members.back());
            std::make_heap(members.begin(), members.end(), heap_comparator);
        }
    }
}

const std::pair<HashFunction, uint64_t>& Population::best() const
{
    return members[0];
}

const std::vector<std::pair<HashFunction, uint64_t>>& Population::getMembers() const
{
    return members;
}

std::vector<std::pair<HashFunction, uint64_t>>& Population::getMembers()
{
    return members;
}

std::pair<HashFunction, uint64_t> Population::popHead()
{
    assert(!members.empty());
    std::pop_heap(members.begin(), members.end(), heap_comparator);
    std::pair<HashFunction, uint64_t> head = std::move(members.back());
    members.pop_back();
    return head;
}

Population::Population( uint64_t reserve )
{
    members.reserve(reserve);
}

uint64_t Population::size() const
{
    return members.size();
}

void Population::resize( uint64_t size )
{
    if ( size <= members.size() ) {
        return;
    }

    sort();
    members.erase(members.begin() + size, members.end());
}

void Population::reheap()
{
    std::make_heap(members.begin(), members.end(), heap_comparator);
}

void Population::sort()
{
    std::sort_heap(members.begin(), members.end(), heap_comparator);
}

void Population::reevaluate( ObjectiveFunction& objective_function )
{
    for ( auto& member: members )
        member.second = objective_function(member.first);
    reheap();
}

bool Population::isHeap() const
{
    return std::is_heap(members.begin(), members.end(), heap_comparator);
}
