#pragma once

#include <mutex>
#include <condition_variable>

template<class T>
class CVQueue
{
public:
    explicit CVQueue(uint64_t size)
        : items(size)
    {
    }

    void push(const T& item)
    {
        {
            std::lock_guard<std::mutex> l(lock);
            items[(head + size) % items.size()] = item;
            size++;
        }
        cv.notify_one();
    }

    T pop()
    {
        std::unique_lock<std::mutex> l(lock);
        cv.wait(l, [&]() { return size != 0; });
        size--;
        return items[(head++) % items.size()];
    }

private:
    std::mutex lock;
    std::condition_variable cv;
    std::vector<T> items;
    uint64_t head = 0;
    uint64_t size = 0;
};