#include "ParallelLocalSearch.h"
#include <cstdint>

std::pair<HashFunction, uint64_t> ParallelLocalSearch::operator()( const HashFunction& in ) const
{
    assert(in.getOutputBits() == 1);
    std::unique_lock<std::mutex> lock(work_mutex);
    work_cv.wait(lock, [&]()
    { return !has_hash && num_running == 0; });
    processing = true;
    hash_function = in;
    has_hash = true;
    best_score = objective_function(in);
    // Start the workers
    work_cv.notify_all();
    // Wait for the workers to all complete
    work_cv.wait(lock, [&]()
    { return num_running == 0 && !processing; });
    has_hash = false;
    return {hash_function, objective_function(hash_function)};
}

ParallelLocalSearch::ParallelLocalSearch( ObjectiveFunction& function,
                                          uint64_t num_threads )
        : objective_function(function)
        , num_threads(num_threads)
        , num_complete(num_threads)
{
    for ( uint64_t i = 0; i < num_threads; i++ )
        thread_pool.emplace_back([i, this]()
                                 {
                                     this->threadFunction(i);
                                 });
}

void ParallelLocalSearch::threadFunction( uint64_t index )
{
    std::unique_lock<std::mutex> lock(work_mutex);
    while ( running ) {
        work_cv.wait(lock, [&]()
        { return (num_running == 0 && !processing) || !running; });
        if ( !running )
            break;

        num_running++;

        uint64_t current_val = UINT64_MAX;
        uint64_t next_val = best_score;
        HashFunction f = hash_function;
        while ( next_val < current_val ) {
            num_complete--;
            lock.unlock();

            current_val = next_val;
            next_val = improve(f, current_val, index);

            lock.lock();
            if ( next_val < best_score ) {
                hash_function = std::move(f);
                best_score = next_val;
            }
            num_complete++;
            if ( num_complete != num_threads )
                loop_cv.wait(lock, [&]()
                { return num_complete == num_threads; });
            else
                loop_cv.notify_all();
            f = hash_function;
            next_val = best_score;
        }

        num_running--;
        if ( num_running == 0 ) {
            processing = false;
            work_cv.notify_all();
        }
    }
}

ParallelLocalSearch::~ParallelLocalSearch()
{
    running = false;
    work_cv.notify_all();
    for ( auto& thread : thread_pool )
        thread.join();
}

uint64_t ParallelLocalSearch::improve( HashFunction& function,
                                       uint64_t current_val,
                                       uint64_t tid )
{
    auto& pla = function.getPLAs().front();
    uint64_t num_bits = function.getInputBits();
    Term best_term;
    Term* best_location = nullptr;
    for ( uint64_t i = tid * num_threads; i < (tid + 1) * num_threads && i < pla.size(); i++ ) {
        Term& term = pla[i];
        const Term old_term = term;
        for ( uint64_t term_bit = 0; term_bit < num_bits; term_bit++ ) {
            for ( uint64_t val = Term::KEEP; val <= Term::DONT_CARE; val++ ) {
                if ( old_term.get(term_bit) != val ) {
                    term.set(term_bit, (Term::BitValue)val);
                    uint64_t score = objective_function(function);
                    if ( score < current_val ) {
                        best_term = term;
                        best_location = &term;
                        current_val = score;
                    }
                }
            }

            if ( old_term.isInit() ) {
                // Check the negated term
                term.flipNegation();
                uint64_t score = objective_function(function);
                if ( score < current_val ) {
                    best_term = term;
                    best_location = &term;
                    current_val = score;
                }

                // Check the empty term
                term.clear();
                score = objective_function(function);
                if ( score < current_val ) {
                    best_term = term;
                    best_location = &term;
                    current_val = score;
                }
            }

            // Restore the original term
            term = old_term;
        }
    }

    // Reapply the best improvement we found
    if ( best_location ) {
        *best_location = best_term;
    }

    return current_val;
}
