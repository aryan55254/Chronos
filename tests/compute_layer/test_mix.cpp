#include <iostream>
#include <vector>
#include <atomic>
#include <chrono>
#include <cmath>
#include "../include/Scheduler.hpp"

// Counters
alignas(64) std::atomic<int> completed_jobs{0};
alignas(64) std::atomic<int> heavy_jobs_done{0};

// --- TASKS ---

// Heavy: Find 500th Prime (~80 microseconds)
void heavy_task(void *arg)
{
    long n = (long)arg;
    int count = 0;
    long num = 2;
    while (count < n)
    {
        bool is_prime = true;
        for (long i = 2; i <= std::sqrt(num); ++i)
        {
            if (num % i == 0)
            {
                is_prime = false;
                break;
            }
        }
        if (is_prime)
            count++;
        num++;
    }
    heavy_jobs_done.fetch_add(1, std::memory_order_relaxed);
    completed_jobs.fetch_add(1, std::memory_order_relaxed);
}

// Light: Almost instant
void light_task(void *arg)
{
    completed_jobs.fetch_add(1, std::memory_order_relaxed);
}

int main()
{
    const int TOTAL_JOBS = 1'000'000;

    // 25% Heavy = 1 in 4 jobs
    // Modulo 4 means indices 0, 4, 8... are heavy.
    const int HEAVY_MODULO = 4;

    std::cout << "\n[Chronos] Starting 25% HEAVY Mix Test...\n";
    std::cout << "------------------------------------------------\n";
    std::cout << "Total Jobs: " << TOTAL_JOBS << "\n";
    std::cout << "Mix:        25% Heavy (250k), 75% Light (750k)\n";

    Scheduler scheduler;
    scheduler.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto start = std::chrono::high_resolution_clock::now();

    Job heavy_job;
    heavy_job.func = heavy_task;
    heavy_job.arg = (void *)500;

    Job light_job;
    light_job.func = light_task;
    light_job.arg = nullptr;

    for (int i = 0; i < TOTAL_JOBS; ++i)
    {
        if (i % HEAVY_MODULO == 0)
        {
            scheduler.Submit(heavy_job);
        }
        else
        {
            scheduler.Submit(light_job);
        }
    }

    // Wait for finish
    while (completed_jobs.load(std::memory_order_relaxed) < TOTAL_JOBS)
    {
        std::this_thread::yield();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seconds = end - start;

    // Throughput of 1 million jobs (some heavy, some light)
    double effective_throughput = TOTAL_JOBS / seconds.count();

    std::cout << "------------------------------------------------\n";
    std::cout << "Time Taken: " << seconds.count() << "s\n";
    std::cout << "Throughput: " << effective_throughput / 1000.0 << " k jobs/sec\n";
    std::cout << "Heavy Done: " << heavy_jobs_done.load() << " (Should be 250000)\n";
    std::cout << "------------------------------------------------\n";

    scheduler.stop();
    return 0;
}