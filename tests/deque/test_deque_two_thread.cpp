#include "../../include/Deque.hpp"
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

struct Stats
{
    std::atomic<uint64_t> executed{0};
    std::atomic<uint64_t> stolen{0};
};

static void work(void *arg)
{
    auto *stats = static_cast<Stats *>(arg);
    stats->executed.fetch_add(1, std::memory_order_relaxed);
}

int main()
{
    Deque dq(1024);
    Stats stats;

    constexpr int JOBS = 1000;

    for (int i = 0; i < JOBS; ++i)
    {
        dq.push_bottom(Job{work, &stats});
    }

    std::thread thief([&]
                      {
        Job job;
        while (stats.executed.load() < JOBS) {
            if (dq.steal_top(job)) {
                stats.stolen.fetch_add(1, std::memory_order_relaxed);
                job.func(job.arg);
            }
        } });

    Job job;
    while (stats.executed.load() < JOBS)
    {
        if (dq.pop_bottom(job))
        {
            job.func(job.arg);
        }
    }

    thief.join();

    assert(stats.executed.load() == JOBS);

    std::cout << "[OK] two-thread steal test\n";
    std::cout << "Executed : " << stats.executed.load() << "\n";
    std::cout << "Stolen   : " << stats.stolen.load() << "\n";

    return 0;
}
