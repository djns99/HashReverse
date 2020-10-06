#pragma once

#include <cstdint>
#include <random>

template<class T>
class RankDistribution
{
public:
    RankDistribution( T num_elements )
            : num_elements(num_elements)
            , dist(0, num_elements * (num_elements + 1) / 2 - 1)
    {

    }

    template<class URBG>
    T operator()( URBG& urbg )
    {
        return binarySearchRank(dist(urbg));
    }

private:
    uint64_t binarySearchRank( uint64_t gen )
    {
        uint64_t top = num_elements;
        uint64_t bottom = 1;

        while ( top != bottom ) {
            uint64_t half_diff = (top - bottom) / 2;
            uint64_t mid = bottom + half_diff;
            if ( (mid * (mid + 1)) / 2 <= gen ) {
                bottom = mid + 1;
            } else {
                top = mid;
            }
        }

        return num_elements - bottom;
    }

    T num_elements;
    std::uniform_int_distribution<T> dist;
};