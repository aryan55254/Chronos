#include "../include/Workers.hpp"
#include "../include/Scheduler.hpp"
#include <immintrin.h> // For _mm_pause()
#include <random>

Worker::Worker(int id, Scheduler *scheduler)
    : id(id),
      deque(4096),
      mailbox(4096),
      scheduler(scheduler),
      running(true) {};

void Worker::start()
{
    thread = std::thread(&Worker::run, this);
}
void Worker::stop()
{
    running.store(false, std::memory_order_relaxed);
    if (thread.joinable())
    {
        thread.join();
    }
}
void Worker::run()
{
    Job job;

    // Thread local Random Number Generator , each thread gets its own
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());

    int num_workers = scheduler->get_worker_count();
    std::uniform_int_distribution<int> dist(0, num_workers - 1);

    while (running.load(std::memory_order_relaxed))
    {
        if (deque.pop_bottom(job))
        {
            job.func(job.arg);
            continue;
        }
        if (mailbox.pop_tail(job))
        {
            if (deque.push_bottom(job))
            {
                continue;
            }
            else
            {
                job.func(job.arg);
                continue;
            }
        }

        // Both queues are empty steal jobs now

        int victim_id = dist(gen);
        if (victim_id != id)
        {
            Worker *victim = scheduler->get_worker(victim_id);
            if (victim->deque.steal_top(job))
            {
                // Stolen jobs are executed immediately
                job.func(job.arg);
                continue;
            }
        }

        // if idle
        _mm_pause();
    }
};