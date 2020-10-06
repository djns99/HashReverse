#include "HashFunction.h"

HashFunction::HashFunction( uint64_t num_input_bits,
                            uint64_t num_output_bits,
                            uint64_t max_terms )
        : num_input_bits(num_input_bits)
        , num_output_bits(num_output_bits)
        , hash_function(num_output_bits, std::vector<Term>(max_terms))
{
}

uint64_t HashFunction::num_calls = 0;
uint64_t HashFunction::operator()( uint64_t input ) const
{
    num_calls++;
    uint64_t output = 0x0;
    for ( uint64_t i = 0; i < num_output_bits; i++ ) {
        uint64_t set = 0;
        for ( const Term& term : hash_function[i] ) {
            if ( term(input) ) {
                set = 1;
                break;
            }
        }
        output |= set << i;
    }
    return output;
}
