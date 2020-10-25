#include "ObjectiveFunction.h"

std::atomic<uint64_t> ObjectiveFunction::num_calls = {0};

void ObjectiveFunction::resetCalls()
{
    num_calls = 0;
}

uint64_t ObjectiveFunction::getNumCalls()
{
    return num_calls;
}

void ObjectiveFunction::incrementCalls()
{
    num_calls.fetch_add(1, std::memory_order_relaxed);
}
