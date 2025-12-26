#include "../include/Deque.hpp"
#include <stdexcept>
#include <iostream>
/*
constructor initializes indices and prepares power-of-two masking for the ring buffer
*/
Deque::Deque(size_t capacity) : capacity(capacity), top(0), bottom(0)
{
    if (capacity == 0 || (capacity & (capacity - 1)) != 0)
    {
        throw std::invalid_argument("Capacity must be a power of 2");
    }
    buffer = new Job[capacity];
    mask = capacity - 1;
}
/*
destructor
*/
Deque::~Deque()
{
    delete[] buffer;
}
/*
- ONLY called by owner thread
- flow :atomic variables copy initiazed to use them ->  overflow checked -> index updated -> job added
*/
bool Deque::push_bottom(Job job)
{
    size_t t = top.load(std::memory_order_relaxed);
    size_t b = bottom.load(std::memory_order_relaxed);
    if ((b - t) >= capacity)
    {
        std::cout << "can't add job , deque overflow" << std::endl;
        return false;
    }
    else
    {
        size_t index = b & mask;
        buffer[index] = job;
        // here memory order release means that buffere index job must be availale to read to other threads before the bottom's  incremented value is available to be read
        bottom.store(b + 1, std::memory_order_release);
        return true;
    }
}
/*
- ONLY called by owner thread except one edge case when bottom = top
- flow :atomic variables copy initiazed to use them ->  bottom is decremented first before checking because by the time we valdate the thieves can change top and that will make the older check reduntanta and can lead to errors -> underflow checked -> edge case solved by CAS -> job popped
*/
bool Deque::pop_bottom(Job &out)
{
    size_t b = bottom.load(std::memory_order_relaxed);
    bottom.store(b - 1, std::memory_order_relaxed);
    // used acquire here because top also has chances of being read by other threads
    size_t t = top.load(std::memory_order_acquire);
    size_t new_b = b - 1;
    if (new_b < t)
    {
        bottom.store(b, std::memory_order_relaxed);

        return false;
    }
    if (new_b == t)
    // handle the edge case with compare and swap the memory ordering on success, acq_rel ensures the owner observes the job correctly and publishes ownership; on failure, relaxed ordering is sufficient
    {
        if (!top.compare_exchange_strong(t, t + 1, std::memory_order_acq_rel,
                                         std::memory_order_relaxed))
        {
            bottom.store(t + 1, std::memory_order_relaxed);
            return false;
        }
    }
    out = buffer[new_b & mask];
    return true;
}
bool Deque::steal_top(Job &out)
// CAS is required here because multiple thief threads may race to steal the same job;
// compare_exchange ensures only one thief can claim ownership of the task.

{
    size_t b = bottom.load(std::memory_order_acquire);
    size_t t = top.load(std::memory_order_acquire);
    Job job = buffer[t & mask];

    if (t >= b)
    {
        return false;
    }
    if (!top.compare_exchange_strong(t, t + 1, std::memory_order_acq_rel,
                                     std::memory_order_relaxed))
    {
        return false;
    }
    out = job;
    return true;
}