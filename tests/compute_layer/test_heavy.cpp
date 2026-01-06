#include <iostream>
#include <vector>
#include <atomic>
#include <chrono>
#include <cmath>
#include "../include/Scheduler.hpp"

// Shared counter to verify completion
alignas(64) std::atomic<int> completed_jobs{0};

// --- THE HEAVY WORK ---
// Finds the Nth prime number. This is computationally expensive.
// N = 1000 takes ~50-100 microseconds.
void find_nth_prime(void *arg)
{
    long n = (long)arg; // Cast the pointer back to integer

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
        {
            count++;
        }
        num++;
    }

    // Mark job as done
    completed_jobs.fetch_add(1, std::memory_order_relaxed);
}

int main()
{
    // 1. Setup
    // We will submit fewer jobs because they are heavy!
    const int NUM_JOBS = 1000000; // 1M Heavy Jobs
    const int PRIME_TARGET = 500; // Find the 500th prime (Moderate Load)

    std::cout << "\n[Chronos] Starting HEAVY Compute Test...\n";
    std::cout << "------------------------------------------------\n";
    std::cout << "Workload: Find " << PRIME_TARGET << "th Prime Number per job\n";

    Scheduler scheduler;
    scheduler.start();

    // Warmup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "[Test] Submitting " << NUM_JOBS << " heavy jobs...\n";
    auto start = std::chrono::high_resolution_clock::now();

    // 2. Submit Loop
    Job job;
    job.func = find_nth_prime;
    job.arg = (void *)((long)PRIME_TARGET); // Hacky pass-by-value

    for (int i = 0; i < NUM_JOBS; ++i)
    {
        scheduler.Submit(job);
    }

    // 3. Wait
    while (completed_jobs.load(std::memory_order_relaxed) < NUM_JOBS)
    {
        std::this_thread::yield();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seconds = end - start;

    // 4. Results
    std::cout << "------------------------------------------------\n";
    std::cout << "[Result] Time Taken: " << seconds.count() << "s\n";
    std::cout << "[Result] Throughput: " << (NUM_JOBS / seconds.count()) << " jobs/sec\n";
    std::cout << "------------------------------------------------\n";

    if (completed_jobs.load() == NUM_JOBS)
    {
        std::cout << "[PASSED] All heavy jobs finished.\n";
    }

    scheduler.stop();
    return 0;
}