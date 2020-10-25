/**
 * Daniel Stokes 1331334
 */
#pragma once
#include "ObjectiveFunction.h"
#include <fstream>

template<class URBG>
class FileReaderObjectiveFunction : public ObjectiveFunction
{
public:
    explicit FileReaderObjectiveFunction( const std::string& filename, URBG& urbg, uint64_t test_num_samples = 0 )
        : urbg(std::uniform_int_distribution<uint64_t>{}(urbg))
    {
        std::ifstream file(filename, std::ios_base::in);
        if(!file)
            throw std::invalid_argument("Bad file path");
        char comma = 0x1;
        if(!(file >> std::dec >> input_bits >> comma >> output_bits >> comma >> num_samples))
            throw std::runtime_error("Failed to read file header");
        if(input_bits == 0 || output_bits == 0)
            throw std::invalid_argument("Incorrect file format");
        // Read all samples
        for ( uint64_t i = 0; i < num_samples; i++ ) {
            uint64_t input, output;
            if(!(file >> std::dec >> input >> comma >> std::hex >> output >> std::dec))
                throw std::runtime_error("Failed to read file - line " + std::to_string(i));
            hashes.emplace_back(input, output);
        }

        // Overwrite the num samples if we should only use a subset of them
        if ( test_num_samples != 0 )
            num_samples = std::min(num_samples, test_num_samples);
    }

    uint64_t operator()( const HashFunction& function ) override
    {
        incrementCalls();
        uint64_t score = 0;
        for(uint64_t i = 0; i < num_samples; i++)
        {
            std::uniform_int_distribution<uint64_t> dist(i, hashes.size() - 1);
            uint64_t index = i;
            if(num_samples != hashes.size())
                index = dist(urbg);
            auto& selected = hashes[index];
            score += __builtin_popcountll(function(selected.first) ^ selected.second);
            if(index != i)
                std::swap(selected, hashes[i]);
        }
        return score;
    }

    std::unique_ptr<ObjectiveFunction> clone() override
    {
        return std::make_unique<FileReaderObjectiveFunction>(*this);
    }

    [[nodiscard]] uint64_t getInputBits() const
    {
        return input_bits;
    }

    [[nodiscard]] uint64_t getOutputBits() const
    {
        return output_bits;
    }

    FileReaderObjectiveFunction(FileReaderObjectiveFunction& other)
        : ObjectiveFunction(other)
        , input_bits(other.input_bits)
        , urbg(std::uniform_int_distribution<uint64_t>{}(other.urbg))
        , hashes(other.hashes)
    {
    }
private:
    uint64_t input_bits;
    URBG urbg;
    std::vector<std::pair<uint64_t, uint64_t>> hashes;
};



