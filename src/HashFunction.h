#pragma once
#include "Term.h"
#include <array>
#include <cstdint>
#include <vector>
#include <limits>

class HashFunction
{
public:
    HashFunction(uint64_t num_input_bits, uint64_t num_output_bits, uint64_t max_terms);
    HashFunction(const HashFunction&) = default;
    HashFunction(HashFunction&&) = default;
    HashFunction& operator=(const HashFunction& other)
    {
        hash_function = other.hash_function;
        return *this;
    }

    HashFunction& operator=(HashFunction&& other) noexcept
    {
        hash_function = std::move(other.hash_function);
        return *this;
    }

    [[nodiscard]] uint64_t operator()(uint64_t input) const;

    [[nodiscard]] uint64_t getInputBits() const
    {
        return num_input_bits;
    }

    [[nodiscard]] uint64_t getOutputBits() const
    {
        return num_output_bits;
    }

    std::vector<std::vector<Term>>& getPLAs()
    {
        return hash_function;
    }
private:
    const uint64_t num_input_bits;
    const uint64_t num_output_bits;
    std::vector<std::vector<Term>> hash_function;
};
