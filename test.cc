/**
 * Daniel Stokes 1331334
 */
#include <map>
#include "src/RecombinationOperators/WeightedCrossover.h"
#include "src/LocalSearch.h"
#include "src/ParallelLocalSearch.h"

#define ASSERT( pred ) do{ if( !(pred) ) { printf("Failed at line %d in function %s: ASSERT(%s)\n\n", __LINE__, __PRETTY_FUNCTION__, #pred); fflush(stdout); abort(); } } while (false)

void test_rank_distribution()
{
    RankDistribution<uint64_t> dist(10);
    std::map<uint64_t, uint64_t> occurances;
    std::mt19937_64 twister(0xD5);
    for ( uint64_t i = 0; i < 10000; i++ ) {
        occurances[dist(twister)]++;
    }

    for ( auto& item : occurances )
        std::cout << item.first << " occurred " << item.second << "/10000 times\n";
}

HashFunction create_cleared( uint64_t in_bits,
                             uint64_t out_bits,
                             uint64_t terms,
                             Term::BitValue initial )
{
    HashFunction new_h(in_bits, out_bits, terms);
    if ( initial != Term::ALWAYS_FALSE )
        for ( auto& pla : new_h.getPLAs() )
            for ( auto& term : pla )
                for ( uint64_t i = 0; i < new_h.getInputBits(); i++ )
                    term.set(i, initial);
    return new_h;
}

template<class Search>
void n_bit_hash_search( const HashFunction& reference,
                        ObjectiveFunction& objective_function,
                        Search& search,
                        Term::BitValue initial_value )
{
    HashFunction new_h = create_cleared(reference.getInputBits(),
                                        reference.getOutputBits(),
                                        reference.getMaxTerms(),
                                        initial_value);

    uint64_t old_score = objective_function(new_h);
    auto searched = search(new_h);

    uint64_t new_score = objective_function(searched.first);
    ASSERT (new_score <= old_score);
    std::cout << new_score << " vs. " << old_score << std::endl;
}

void test_local_search()
{
    std::cout << "Sequential Local Search" << std::endl;
    std::mt19937_64 twister(0xD5);

    HashFunction size_1(1, 1, 1);
    std::vector<HashFunction> test_configs;
    test_configs.push_back(size_1);
    for ( uint64_t j = 0; j < 2; j++ ) {
        for ( uint64_t i = Term::KEEP; i <= Term::DONT_CARE; i++ ) {
            size_1.getPLAs()[0][0].set(0, (Term::BitValue)i);
            test_configs.push_back(size_1);
        }
        size_1.getPLAs()[0][0].flipNegation();
    }

    HashFunction and_gate(2, 1, 1);
    and_gate.getPLAs()[0][0].set(0, Term::KEEP);
    and_gate.getPLAs()[0][0].set(1, Term::KEEP);
    test_configs.push_back(and_gate);

    HashFunction xor_gate(2, 1, 2);
    xor_gate.getPLAs()[0][0].set(0, Term::KEEP);
    xor_gate.getPLAs()[0][0].set(1, Term::INVERT);
    xor_gate.getPLAs()[0][0].set(1, Term::KEEP);
    xor_gate.getPLAs()[0][0].set(0, Term::INVERT);
    test_configs.push_back(xor_gate);

    for(uint64_t i = Term::ALWAYS_FALSE; i < Term::DONT_CARE; i++) {
        HashFunction cleared = create_cleared(16, 16, 16, (Term::BitValue)i);
        test_configs.push_back(cleared);
    }

    uint64_t i = 0;
    for ( auto& hash_function : test_configs ) {
        std::cout << "Running config " << ++i << std::endl;

        CachedObjectiveFunction objective_function(hash_function,
                                                   1ull << std::min(hash_function.getInputBits(), 16ul),
                                                   twister);
        LocalSearch first_search(LocalSearchMode::FIRST_IMPROVEMENT, objective_function);

        n_bit_hash_search(hash_function, objective_function, first_search, Term::ALWAYS_FALSE);
        n_bit_hash_search(hash_function, objective_function, first_search, Term::DONT_CARE);
        n_bit_hash_search(hash_function, objective_function, first_search, Term::INVERT);
        n_bit_hash_search(hash_function, objective_function, first_search, Term::KEEP);

        LocalSearch best_search(LocalSearchMode::BEST_IMPROVEMENT, objective_function);

        n_bit_hash_search(hash_function, objective_function, best_search, Term::ALWAYS_FALSE);
        n_bit_hash_search(hash_function, objective_function, best_search, Term::DONT_CARE);
        n_bit_hash_search(hash_function, objective_function, best_search, Term::INVERT);
        n_bit_hash_search(hash_function, objective_function, best_search, Term::KEEP);
    }
}

void test_parallel_local_search()
{
    std::cout << "Parallel Local Search" << std::endl;
    std::mt19937_64 twister(0xD5);

    HashFunction size_1(1, 1, 1);
    std::vector<HashFunction> test_configs;
    test_configs.push_back(size_1);
    for ( uint64_t j = 0; j < 2; j++ ) {
        for ( uint64_t i = Term::KEEP; i <= Term::DONT_CARE; i++ ) {
            size_1.getPLAs()[0][0].set(0, (Term::BitValue)i);
            test_configs.push_back(size_1);
        }
        size_1.getPLAs()[0][0].flipNegation();
    }

    HashFunction and_gate(2, 1, 1);
    and_gate.getPLAs()[0][0].set(0, Term::KEEP);
    and_gate.getPLAs()[0][0].set(1, Term::KEEP);
    test_configs.push_back(and_gate);

    HashFunction xor_gate(2, 1, 2);
    xor_gate.getPLAs()[0][0].set(0, Term::KEEP);
    xor_gate.getPLAs()[0][0].set(1, Term::INVERT);
    xor_gate.getPLAs()[0][0].set(1, Term::KEEP);
    xor_gate.getPLAs()[0][0].set(0, Term::INVERT);
    test_configs.push_back(xor_gate);

    for(uint64_t i = Term::ALWAYS_FALSE; i < Term::DONT_CARE; i++) {
        HashFunction cleared = create_cleared(64, 1, 8, (Term::BitValue)i);
        test_configs.push_back(cleared);
    }

    uint64_t i = 0;
    for ( auto& hash_function : test_configs ) {
        std::cout << "Running config " << ++i << std::endl;

        CachedObjectiveFunction objective_function(hash_function,
                                                   1ull << std::min(hash_function.getInputBits(), 16ul),
                                                   twister);
        ParallelLocalSearch search(objective_function, 2);

        n_bit_hash_search(hash_function, objective_function, search, Term::ALWAYS_FALSE);
        n_bit_hash_search(hash_function, objective_function, search, Term::DONT_CARE);
        n_bit_hash_search(hash_function, objective_function, search, Term::INVERT);
        n_bit_hash_search(hash_function, objective_function, search, Term::KEEP);
    }
}

int main( int argc,
          char** argv )
{
    // test_rank_distribution();
    // test_local_search();
    test_parallel_local_search();
}
