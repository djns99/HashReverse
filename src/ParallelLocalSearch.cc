#include "ParallelLocalSearch.h"
#include "LocalSearch.h"
#include <cstdint>

std::pair<HashFunction, uint64_t> ParallelLocalSearch::operator()( const HashFunction& in ) const
{
    assert(in.getOutputBits() == 1);
    HashResult r{in};
    input_queue.push(&r);
    std::unique_lock<std::mutex> l(r.completion_mutex);
    r.completion_cv.wait(l, [&]()
    { return r.completed; });
    return {std::move(r.output), r.score};
}

ParallelLocalSearch::ParallelLocalSearch(                                          uint64_t num_threads )
        : num_threads(num_threads)
        , input_queue(32)
        , shutting_down(false)
{
    pthread_barrier_init(&sync_barrier, NULL, num_threads);
    for ( uint64_t i = 0; i < num_threads; i++ )
        thread_pool.emplace_back([i, this]()
                                 {
                                     this->threadFunction(i);
                                 });
}

void ParallelLocalSearch::threadFunction( uint64_t index )
{
    HashResult* input = nullptr;
    while ( true ) {
        if ( index == 0 ) {
            input = input_queue.pop();
            if ( input ) {
                sync_best_hash = input->input;
                sync_best_score = UINT64_MAX;
            } else {
                shutting_down = true;
            }
            pthread_barrier_wait(&sync_barrier);
        } else {
            pthread_barrier_wait(&sync_barrier);
        }

        if ( shutting_down.load() )
            break;

        uint64_t current_val = UINT64_MAX;
        uint64_t next_val = sync_best_score;
        HashFunction f = sync_best_hash;

        pthread_barrier_wait(&sync_barrier);

        do {
            current_val = next_val;
            next_val = improve(f, current_val, index);

            if ( next_val <= current_val ) {
                std::lock_guard<std::mutex> l(sync_mutex);
                if ( next_val < sync_best_score ) {
                    sync_best_score = next_val;
                    sync_best_hash = std::move(f);
                }
            }

            pthread_barrier_wait(&sync_barrier);

            f = sync_best_hash;
            next_val = sync_best_score;

            pthread_barrier_wait(&sync_barrier);
        } while ( next_val < current_val );

        if ( index == 0 ) {
            assert(input);
            input->output = std::move(sync_best_hash);
            input->score = sync_best_score;
            input->completion_mutex.lock();
            input->completed = true;
            // Must notify under lock to prevent thread from cleaning up
            input->completion_cv.notify_one();
            input->completion_mutex.unlock();
        }
    }
}

ParallelLocalSearch::~ParallelLocalSearch()
{
    input_queue.push(nullptr);
    for ( auto& thread : thread_pool )
        thread.join();
    pthread_barrier_destroy(&sync_barrier);
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
                    uint64_t score = (*objective_function)(function);
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
                uint64_t score = (*objective_function)(function);
                if ( score < current_val ) {
                    best_term = term;
                    best_location = &term;
                    current_val = score;
                }

                // Check the empty term
                term.clear();
                score = (*objective_function)(function);
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

void ParallelLocalSearch::setObjectiveFunction( ObjectiveFunction& objective_function )
{
    this->objective_function = &objective_function;
}
