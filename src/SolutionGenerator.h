#pragma once
#include "HashFunction.h"

class SolutionGenerator
{
public:
    virtual HashFunction operator()() = 0;
};