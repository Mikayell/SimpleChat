#ifndef THREAD_SAFE_QUEUE
#define THREAD_SAFE_QUEUE

#include "common_headers.h"

template<typename T>
class TSQueue
{

public:
    TSQueue() = default;
    TSQueue(const TSQueue<T>&) = delete;
    virtual ~TSQueue()
    {
        clear();
    }
    void push_back(const T& obj)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _deque.emplace_back(std::move(obj));
    }
    bool empty()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _deque.empty();
    }
    size_t count()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _deque.size();
    }
    const T& front()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _deque.front();
    }
    T pop_front()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto tmp = std::move(_deque.front());
        _deque.pop_front();
        return tmp;
    }

    const T& back()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _deque.back();
    }
    T pop_back()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto tmp = std::move(_deque.back());
        _deque.pop_back();
        return tmp;
    }
    void clear()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _deque.clear();
    }
private:
    std::mutex _mutex;
    std::deque<T> _deque;
};

#endif