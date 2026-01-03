#include "../../include/Mailbox.hpp"
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

// Define a count of jobs
constexpr int JOBS_PER_THREAD = 10000;
constexpr int NUM_PRODUCERS = 4;
constexpr int TOTAL_JOBS = JOBS_PER_THREAD * NUM_PRODUCERS;

int main()
{
    Mailbox mb(65536); // Large capacity to minimize spins
    std::atomic<int> received_count{0};

    // --- PRODUCERS (Simulating Reactor + API threads) ---
    std::vector<std::thread> producers;
    for (int i = 0; i < NUM_PRODUCERS; ++i)
    {
        producers.emplace_back([&mb]()
                               {
            for (int j = 0; j < JOBS_PER_THREAD; ++j)
            {
                // Spin until we successfully push
                // (In a real app, you might back off, but for testing we want stress)
                Job job{nullptr, nullptr};
                while (!mb.push_head(job)) {
                    std::this_thread::yield();
                }
            } });
    }

    // --- CONSUMER (Simulating the Worker) ---
    std::thread consumer([&]()
                         {
        Job out;
        while (received_count.load(std::memory_order_relaxed) < TOTAL_JOBS)
        {
            if (mb.pop_tail(out))
            {
                received_count.fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                // If empty, yield briefly
                std::this_thread::yield();
            }
        } });

    // Join all
    for (auto &t : producers)
        t.join();
    consumer.join();

    // Verify
    assert(received_count.load() == TOTAL_JOBS);

    std::cout << "[OK] Mailbox MPSC test (" << NUM_PRODUCERS << " producers)\n";
    std::cout << "Total Jobs Processed: " << received_count.load() << "\n";

    return 0;
}