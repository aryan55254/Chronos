#include "../include/Scheduler.hpp"
#include <immintrin.h>
#include <random>

Scheduler::Scheduler() : num_workers(4)
{
    for (size_t i = 0; i < num_workers; ++i)
    {
        workers[i] = new Worker(i, this);
    }
}

Scheduler::~Scheduler()
{
    stop();
    for (size_t i = 0; i < num_workers; ++i)
    {
        delete workers[i];
        workers[i] = nullptr;
    }
}
void Scheduler::start()
{
    for (size_t i = 0; i < num_workers; ++i)
    {
        workers[i]->start();
    }
}
void Scheduler::stop()
{
    for (size_t i = 0; i < num_workers; ++i)
    {
        if (workers[i])
        {
            workers[i]->stop();
        }
    }
}
// adatptive backoff based submission with random distribution
void Scheduler::Submit(Job job)
{
    if (num_workers == 0)
        return;
    static thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, num_workers - 1);
    size_t index = dist(gen);

    Worker *target = workers[index];
    int spins = 0;
    while (!target->mailbox.push_head(job))
    {

        if (spins < 10)
        {
            // Aggressive Spin
            spins++;
        }
        else if (spins < 500)
        {
            // Polite Spin
            _mm_pause();
            spins++;
        }
        else
        {
            //  Surrender
            std::this_thread::yield();
            spins = 0; // Reset to try spinning again after wake up
        }
    }
}