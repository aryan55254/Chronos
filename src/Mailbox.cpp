#include "../include/Mailbox.hpp"
#include <stdexcept>
#include <immintrin.h> // Required for _mm_pause()

/*
constructor initializes indices and prepares power-of-two masking for the ring buffer
*/
Mailbox::Mailbox(size_t capacity) : capacity(capacity), Head(0), Tail(0)
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
Mailbox::~Mailbox()
{
    delete[] buffer;
}
/*
- can be called by the epoll reactor and the main api thread or threads if it is multiple
- flow :atomic variables copy initiazed to use them ->  overflow checked -> index updated -> job added
*/
bool Mailbox::push_head(Job job)
{
    // what ahppens here is if a thread comes and the producer_lock flag is false we go trhough this and go to the main function and lock this behind now  new thread comes sees it is locked then it gets into the while loop until the thread doing the function turns lock off
    // the memory ordering acquired here makes sure that nothing in the function forward does anything unless this lock is on
    while (producer_lock.test_and_set(std::memory_order_acquire))
    {
        _mm_pause();
    }
    size_t h = Head.load(std::memory_order_relaxed); // the thread that comes in owns the lock so this is relaxed
    size_t t = Tail.load(std::memory_order_acquire); // the thread that comes in needs to make sure it sees correct value of tail so we use acquired here

    if ((h - t) >= capacity)
    {
        producer_lock.clear(std::memory_order_release);
        return false;
    }
    buffer[h & mask] = job;
    Head.store(h + 1, std::memory_order_release);
    producer_lock.clear(std::memory_order_release);
    return true;
}
/*
- ONLY called by owner thread except one edge case when bottom = top
- flow :atomic variables copy initiazed to use them ->  bottom is decremented first before checking because by the time we valdate the thieves can change top and that will make the older check reduntanta and can lead to errors -> underflow checked -> edge case solved by CAS -> job popped
*/
bool Mailbox::pop_tail(Job &out)
{
    size_t h = Head.load(std::memory_order_acquire); // used acquire here because top also has chances of being read by other threads
    size_t t = Tail.load(std::memory_order_relaxed);
    if (h <= t)
    {
        return false;
    }

    out = buffer[t & mask];
    Tail.store(t + 1, std::memory_order_release); // release here so that others threads know before reading this that out is published
    return true;
}
