#pragma once

#include "HashFunction.h"
#include <algorithm>
#include <immintrin.h>
#include <iterator>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>
#include <map>

struct CountingIterator : std::iterator<std::forward_iterator_tag, uint64_t>
{
    uint64_t i = 0;

    explicit CountingIterator( uint64_t i = 0 )
            : i(i)
    {
    }

    CountingIterator& operator++()
    {
        i++;
        return *this;
    }

    uint64_t operator*() const
    {
        return i;
    }

    bool operator!=( const CountingIterator& other ) const
    {
        return i != other.i;
    }
};

class ObjectiveFunction
{
public:
    virtual uint64_t operator()( const HashFunction& function ) = 0;

    ObjectiveFunction( uint64_t output_bits,
                       uint64_t num_samples )
            : output_bits(output_bits)
            , num_samples(num_samples)
    {

    }

    [[nodiscard]] double normalize( uint64_t score ) const
    {
        return (double)score / (double)(num_samples * output_bits);
    }

protected:
    uint64_t output_bits;
    uint64_t num_samples;
};

class CachedObjectiveFunction : public ObjectiveFunction
{
public:
    template<class URBG>
    explicit CachedObjectiveFunction( const HashFunction& reference_function,
                                      uint64_t num_samples,
                                      URBG&& urbg )
            : ObjectiveFunction(reference_function.getOutputBits(), num_samples)
    {
        uint64_t max_input = ~0u >> (64u - reference_function.getInputBits());
        if ( num_samples >= max_input ) {
            for ( uint64_t i = 0; i <= max_input; i++ )
                samples.emplace(i, reference_function(i));
        } else if ( num_samples >= max_input / 2 ) {
            std::vector<uint64_t> values(num_samples);
            std::sample(CountingIterator{0},
                        CountingIterator{max_input},
                        values.begin(),
                        num_samples,
                        std::forward<URBG>(urbg));
            for ( auto& val : values )
                samples.emplace(val, reference_function(val));
        } else {
            std::uniform_int_distribution<uint64_t> dist(0, max_input);
            while ( samples.size() < num_samples ) {
                uint64_t val = dist(urbg);
                auto res = samples.try_emplace(val, 0);
                if ( res.second )
                    res.first->second = reference_function(val);
            }
        }

        // Add some common must have cases
        for ( uint64_t i = 0; i < reference_function.getInputBits(); i++ ) {
            samples.emplace(1ull << i, reference_function(i));
            uint64_t all_ones = ~0ull >> (63 - i);
            samples.emplace(all_ones, reference_function(all_ones));
        }
        samples.emplace(0, reference_function(0));
        this->num_samples = samples.size();
    }

    uint64_t operator()( const HashFunction& function ) override
    {
        uint64_t total_distance = 0x0;
        // Return the sum of the Hamming distances for each sample
        for ( auto& sample : samples )
            total_distance += __builtin_popcountll(sample.second ^ function(sample.first));

        return total_distance;
    }

private:
    std::map<uint64_t, uint64_t> samples;
};

template<class URBG>
class UncachedObjectiveFunction : public ObjectiveFunction
{
public:
    UncachedObjectiveFunction( const HashFunction& reference_function,
                               uint64_t num_samples,
                               URBG& urbg )
            : ObjectiveFunction(reference_function.getOutputBits(), num_samples)
            , reference_function(reference_function)
            , num_samples(num_samples)
            , dist(0, ~0ull >> (64u - reference_function.getInputBits()))
            , urbg(urbg)
    {
    }

    uint64_t operator()( const HashFunction& function ) override
    {
        uint64_t count = 0;
        if ( num_samples > dist.max() ) {
            for ( uint64_t i = 0; i < num_samples; i++ )
                count += __builtin_popcountll(function(i) ^ reference_function(i));
        } else if ( num_samples >= dist.max() / 2 && num_samples <= dist.max() ) {
            std::vector<uint64_t> values(num_samples);
            std::sample(CountingIterator{0}, CountingIterator{dist.max() + 1}, values.begin(), num_samples, urbg);
            for ( auto& val : values ) {
                count += __builtin_popcountll(function(val) ^ reference_function(val));
            }
        } else {
            for ( uint64_t i = 0; i < num_samples; i++ ) {
                uint64_t val = dist(urbg);
                count += __builtin_popcountll(function(val) ^ reference_function(val));
            }
        }
        return count;
    }

private:
    const HashFunction& reference_function;
    uint64_t num_samples;
    std::uniform_int_distribution<uint64_t> dist;
    URBG& urbg;
};