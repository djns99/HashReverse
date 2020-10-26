/**
 * Daniel Stokes 1331334
 */
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
    static void incrementCalls();
    static void resetCalls();
    static uint64_t getNumCalls();

    ObjectiveFunction() = default;
    ObjectiveFunction( uint64_t output_bits,
                       uint64_t num_samples )
            : output_bits(output_bits)
            , num_samples(num_samples)
    {
    }

    virtual ~ObjectiveFunction() = default;

    [[nodiscard]] double normalize( uint64_t score ) const
    {
        if(score >= num_samples * output_bits)
            return 1.0;
        return (double)score / (double)(num_samples * output_bits);
    }

    // Must be called in a thread safe manner
    // Returns an object that can be used by a different thread
    virtual std::unique_ptr<ObjectiveFunction> clone() = 0;


protected:
    uint64_t output_bits = 0;
    uint64_t num_samples = 0;
    static std::atomic<uint64_t> num_calls;
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
            samples.emplace(1ull << i, reference_function(1ull << i));
            samples.emplace(~(1ull << i) & max_input, reference_function(~(1ull << i) & max_input));
            uint64_t all_ones = ~0ull >> (63 - i);
            samples.emplace(all_ones, reference_function(all_ones));
            samples.emplace(~all_ones & max_input, reference_function(~all_ones & max_input));
        }
        samples.emplace(0, reference_function(0));
        this->num_samples = samples.size();
    }

    uint64_t operator()( const HashFunction& function ) override
    {
        incrementCalls();
        uint64_t total_distance = 0x0;
        // Return the sum of the Hamming distances for each sample
        for ( auto& sample : samples )
            total_distance += __builtin_popcountll(sample.second ^ function(sample.first));

        return total_distance;
    }

    std::unique_ptr<ObjectiveFunction> clone() override
    {
        return std::make_unique<CachedObjectiveFunction>(*this);
    }

private:
    std::unordered_map<uint64_t, uint64_t> samples;
};

template<class URBG, bool ALWAYS_CHECK = true>
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
            , urbg(std::uniform_int_distribution<uint64_t>{}(urbg))
    {
        if(ALWAYS_CHECK) {
            uint64_t max_input = ~0u >> (64u - reference_function.getInputBits());
            // Check some common must have cases
            for ( uint64_t i = 0; i < reference_function.getInputBits(); i++ ) {
                always_check.emplace(1ull << i, reference_function(1ull << i));
                always_check.emplace(~(1ull << i) & max_input, reference_function(~(1ull << i) & max_input));
                uint64_t all_ones = ~0ull >> (63 - i);
                always_check.emplace(all_ones, reference_function(all_ones));
                always_check.emplace(~all_ones & max_input, reference_function(~all_ones & max_input));
            }
            always_check.emplace(0, reference_function(0));
            this->num_samples += always_check.size();
        }
    }

    uint64_t operator()( const HashFunction& function ) override
    {
        incrementCalls();
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

        if( ALWAYS_CHECK ) {
            for ( const std::pair<const uint64_t, uint64_t>& check_pair : always_check )
                count += __builtin_popcountll(function(check_pair.first) & check_pair.second);
        }

        return count;
    }

    std::unique_ptr<ObjectiveFunction> clone() override
    {
        auto temp = std::make_unique<UncachedObjectiveFunction>(*this);
        temp->urbg = URBG(std::uniform_int_distribution<uint64_t>{}(urbg));
        return temp;
    }

private:
    const HashFunction& reference_function;
    uint64_t num_samples;
    std::uniform_int_distribution<uint64_t> dist;
    URBG urbg; // Store a copy so we can use the clone function to make thread safe versions

    std::unordered_map<uint64_t, uint64_t> always_check;
};

template<class URBG>
using TrueUncachedObjectiveFunction = UncachedObjectiveFunction<URBG, false>;