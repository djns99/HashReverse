/**
 * Daniel Stokes 1331334
 */
#pragma once

#include "../HashFunction.h"

class SolutionGenerator
{
public:
    virtual HashFunction operator()(const MemeticAlgorithm&) = 0;
    virtual ~SolutionGenerator() = default;
};