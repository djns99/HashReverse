#pragma once

#include "../HashFunction.h"

class SolutionGenerator
{
public:
    virtual HashFunction operator()(const MemeticAlgorithm&) = 0;
};