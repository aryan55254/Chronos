#include <iostream>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>
#include <cassert>
#include "../include/Scheduler.hpp"

// =========================================================================
// CHRONOS COMPUTE LAYER INTEGRITY TEST
// =========================================================================
// Goals:
// 1. Correctness: Ensure exactly N jobs are executed (no drops, no dupes).
// 2. Concurrency: Ensure multiple threads contribute to the work.
// 3. Throughput: Measure raw dispatch/execution speed.
// =========================================================================

// Shared atomic counter to verify data integrity across threads
// alignas(64) prevents "false sharing" cache thrashing during the test
alignas(64) std::atomic<int> global_counter{0};

// The Payload: A simple task that increments the counter
void atomic_increment_task(void *arg)
{
    global_counter.fetch_add(1, std::memory_order_relaxed);
}

int main()
{
    // Configuration
    const int NUM_WORKERS = 4;
    const int TOTAL_JOBS = 10'000'000; // 10 Million Jobs

    std::cout << "\n[Chronos] Starting Compute Layer Test...\n";
    std::cout << "------------------------------------------------\n";
    std::cout << "Workers: " << NUM_WORKERS << "\n";
    std::cout << "Jobs:    " << TOTAL_JOBS << "\n";

    // 1. Initialize Scheduler
    // This creates the Workers, Deques, and Mailboxes
    Scheduler scheduler;

    // 2. Start the Engine (Spawns threads)
    scheduler.start();

    // Warmup: Give threads 100ms to spin up and hit their "hot loops"
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "[Test] Submitting jobs...\n";
    auto start_time = std::chrono::high_resolution_clock::now();

    // 3. High-Velocity Submission Loop
    // This stresses the "Scheduler::Submit" and "Mailbox::push_head" logic
    Job job;
    job.func = atomic_increment_task;
    job.arg = nullptr;

    for (int i = 0; i < TOTAL_JOBS; ++i)
    {
        scheduler.Submit(job);
    }

    // 4. Synchronization Barrier
    // Wait until all jobs are drained.
    // In a real app, we'd use Futures. Here, we spin-wait on the atomics.
    while (global_counter.load(std::memory_order_relaxed) < TOTAL_JOBS)
    {
        // Yield to let workers use the CPU
        std::this_thread::yield();
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    // 5. Validation & Statistics
    std::chrono::duration<double> elapsed = end_time - start_time;
    double seconds = elapsed.count();
    double throughput = TOTAL_JOBS / seconds;

    std::cout << "------------------------------------------------\n";
    std::cout << "[Result] Time Taken: " << seconds << "s\n";
    std::cout << "[Result] Throughput: " << (throughput / 1'000'000.0) << " M jobs/sec\n";

    if (global_counter.load() == TOTAL_JOBS)
    {
        std::cout << "[PASSED] Integrity Check: All " << TOTAL_JOBS << " jobs executed.\n";
    }
    else
    {
        std::cerr << "[FAILED] Integrity Check: Expected " << TOTAL_JOBS
                  << " but got " << global_counter.load() << "\n";
        return 1;
    }

    scheduler.stop();
    return 0;
}