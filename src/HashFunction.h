#pragma once

#include "Term.h"
#include <array>
#include <cstdint>
#include <limits>
#include <vector>
#include <atomic>

class HashFunction
{
public:
    HashFunction() = default;

    HashFunction( uint64_t num_input_bits,
                  uint64_t num_output_bits,
                  uint64_t max_terms );

    HashFunction( const HashFunction& ) = default;

    HashFunction( HashFunction&& ) = default;

    HashFunction& operator=( const HashFunction& other ) = default;

    HashFunction& operator=( HashFunction&& other ) noexcept = default;

    [[nodiscard]] uint64_t operator()( uint64_t input ) const;

    bool operator==( const HashFunction& other ) const
    {
        if ( other.num_input_bits != num_input_bits || other.num_output_bits != num_output_bits ||
             other.getMaxTerms() != getMaxTerms() )
            return false;
        for ( uint64_t pla = 0; pla < num_output_bits; pla++ )
            for ( uint64_t term = 0; term < getMaxTerms(); term++ )
                if ( hash_function[pla][term].value_lo != other.hash_function[pla][term].value_lo ||
                     hash_function[pla][term].value_hi != other.hash_function[pla][term].value_hi )
                    return false;
        return true;
    }

    bool operator!=( const HashFunction& other ) const
    {
        return !(*this == other);
    }

    [[nodiscard]] uint64_t getInputBits() const
    {
        return num_input_bits;
    }

    [[nodiscard]] uint64_t getOutputBits() const
    {
        return num_output_bits;
    }

    [[nodiscard]] uint64_t getMaxTerms() const
    {
        assert(!hash_function.empty());
        return hash_function.front().size();
    }

    std::vector<std::vector<Term>>& getPLAs()
    {
        return hash_function;
    }

    [[nodiscard]] const std::vector<std::vector<Term>>& getPLAs() const
    {
        return hash_function;
    }

    [[nodiscard]] uint64_t hashcode() const
    {
        uint64_t code = 0;
        for(auto& pla : hash_function) {
            for ( auto& term : pla )
                code ^= murmur64(term.value_hi ^ term.value_lo);
            // Mix the PLA with murmur so identical PLAs for different bits dont have the same effect
            murmur64(code);
        }
        return code;
    }


    static uint64_t getNumCalls()
    {
        return num_calls;
    }

    static void resetCalls()
    {
        num_calls = 0;
    }
private:
    static uint64_t murmur64(uint64_t code) {
        code ^= code >> 33u;
        code *= 0xff51afd7ed558ccdUL;
        code ^= code >> 33u;
        code *= 0xc4ceb9fe1a85ec53UL;
        code ^= code >> 33u;
        return code;
    }

    uint64_t num_input_bits = 0;
    uint64_t num_output_bits = 0;
    std::vector<std::vector<Term>> hash_function{};

    static std::atomic<uint64_t> num_calls;
};
