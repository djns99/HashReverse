#pragma once
#include <cassert>
#include <cstdint>
#include <immintrin.h>

struct Term {
    enum BitValue : uint64_t
    {
        ALWAYS_FALSE = 0x0,
        KEEP = 0x1,
        INVERT = 0x2,
        DONT_CARE = 0x3,
    };

    // Default to 0 so uninitialised dont affect result
    uint64_t value_lo = 0x0;
    uint64_t value_hi = 0x0;

    bool isInit() const
    {
        return value_lo;
    }

    void init()
    {
        if(!isInit())
        {
            value_lo = UINT64_MAX;
            value_hi = UINT64_MAX;
        }
    }

    bool operator()(uint64_t input) const
    {
        const uint64_t input_lo = interleave(input, ~input);
        const uint64_t input_hi = interleave(input >> 32ull, ~(input >> 32ull));
        const uint64_t lo_mask = input_lo & value_lo;
        const uint64_t hi_mask = input_hi & value_hi;
        const uint64_t lo_res = merge(lo_mask);
        const uint64_t hi_res = merge(hi_mask);
        return lo_res == UINT32_MAX && hi_res == UINT32_MAX;
    }

    void set(uint32_t index, BitValue val)
    {
        assert(val != ALWAYS_FALSE);
        init();
        if (index < 32)
        {
            setBitValue(value_lo, index, val);
        }
        else
        {
            setBitValue(value_hi, index - 32, val);
        }
    }

    BitValue get(uint64_t index) const
    {
        if (index < 32)
        {
            return getBitValue(value_lo, index);
        }
        else
        {
            return getBitValue(value_hi, index - 32);
        }
    }

    void clear()
    {
        value_lo = 0x0;
        value_hi = 0x0;
    }

private:
    static void setBitValue(uint64_t& word, uint64_t index, BitValue val)
    {
        assert(index < 32);
        uint64_t mask = 0x3ull << (index * 2);
        word &= ~mask;
        word |= val << (index * 2);
    }

    static BitValue getBitValue(uint64_t word, uint64_t index)
    {
        assert(index < 32);
        return (BitValue)((word >> (index * 2)) & 0x3ull);
    }

    static uint64_t interleave(uint32_t a, uint32_t b)
    {
        return _pdep_u64(a, 0x5555555555555555ull) | _pdep_u64(b, 0xaaaaaaaaaaaaaaaaull);
    }

    static uint64_t merge(uint64_t val)
    {
        return _pext_u64(val, 0x5555555555555555ull) | _pext_u64(val, 0xaaaaaaaaaaaaaaaaull);
    }
};