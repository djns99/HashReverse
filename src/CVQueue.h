#pragma once

#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <cassert>

template<class T>
class CVQueue
{
public:
    explicit CVQueue( uint64_t size )
            : items(size)
            , running(true)
    {
    }

    void push( const T& item )
    {
        assert(items.size());
        assert(running);
        {
            std::unique_lock<std::mutex> l(lock);
            cv_push.wait(l, [&]()
            { return size != items.size() || !running; });
            items[(head + size) % items.size()] = item;
            size++;
        }
        cv_pop.notify_one();
    }

    T pop()
    {
        assert(items.size());
        std::unique_lock<std::mutex> l(lock);
        cv_pop.wait(l, [&]()
        { return size != 0 || !running; });
        if ( !running )
            return T();
        size--;
        cv_push.notify_one();
        return items[(head++) % items.size()];
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> l(lock);
            running = false;
        }
        cv_pop.notify_all();
        cv_push.notify_all();
    }

    void resize( uint64_t new_size )
    {
        assert(size == 0);
        items.resize(new_size);
    }

    void start()
    {
        running = true;
    }

private:
    std::mutex lock;
    std::condition_variable cv_pop;
    std::condition_variable cv_push;
    std::vector<T> items;
    uint64_t head = 0;
    uint64_t size = 0;
    bool running = true;
};