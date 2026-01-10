#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <future>
#include "../include/Scheduler.hpp"

// Light Task
int task_easy() {
    volatile int x = 1 + 1; 
    return x;
}

// Heavy Task (~5-10us of CPU work)
int task_heavy() {
    volatile int val = 1;
    for(int i=0; i<5000; ++i) val += i * val;
    return val;
}

struct Stats {
    double throughput_mps;
    double avg_lat_us;
    double p50_lat_us;
    double p99_lat_us;
};

int main() {
    const size_t TOTAL_JOBS = 10000000; // 10 Million
    const size_t LATENCY_SAMPLES = 100000;

    std::cout << "\n=============================================================\n";
    std::cout << "     CHRONOS BENCHMARK: MIXED 25% HEAVY (10 Million Jobs)    \n";
    std::cout << "=============================================================\n";
    std::cout << "Cores: " << std::thread::hardware_concurrency() << "\n";

    Scheduler scheduler;
    scheduler.start();
    std::cout << "Engine Online.\n\n";

    Stats stats;

    // --- PHASE 1: LATENCY ---
    {
        std::vector<double> latencies;
        latencies.reserve(LATENCY_SAMPLES);

        for (size_t i = 0; i < LATENCY_SAMPLES; ++i) {
            bool is_heavy = (i % 4 == 0); // 25% Heavy
            auto t_start = std::chrono::high_resolution_clock::now();
            
            if(is_heavy) scheduler.submit(task_heavy).get();
            else         scheduler.submit(task_easy).get();
            
            auto t_end = std::chrono::high_resolution_clock::now();
            latencies.push_back(std::chrono::duration<double, std::micro>(t_end - t_start).count());
        }

        std::sort(latencies.begin(), latencies.end());
        double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
        stats.p50_lat_us = latencies[LATENCY_SAMPLES * 0.50];
        stats.p99_lat_us = latencies[LATENCY_SAMPLES * 0.99];
    }

    // --- PHASE 2: THROUGHPUT ---
    {
        const size_t BATCH_SIZE = 100000; 
        size_t jobs_remaining = TOTAL_JOBS;
        auto t_start = std::chrono::high_resolution_clock::now();

        while(jobs_remaining > 0) {
            size_t current = std::min(BATCH_SIZE, jobs_remaining);
            std::vector<std::future<int>> futures;
            futures.reserve(current);

            for (size_t i = 0; i < current; ++i) {
                bool is_heavy = (i % 4 == 0); // 25% Heavy
                if(is_heavy) futures.push_back(scheduler.submit(task_heavy));
                else         futures.push_back(scheduler.submit(task_easy));
            }
            for (auto& f : futures) f.get();

            jobs_remaining -= current;
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        double sec = std::chrono::duration<double>(t_end - t_start).count();
        stats.throughput_mps = (TOTAL_JOBS / 1000000.0) / sec;
    }

    // Output Table
    std::cout << std::left 
              << std::setw(30) << "Scenario" << " | " 
              << std::setw(12) << "Total Jobs" << " | " 
              << std::setw(14) << "Throughput" << " | " 
              << std::setw(10) << "P50 Lat" << " | " 
              << std::setw(10) << "P99 Lat" << "\n" 
              << std::string(90, '-') << "\n"
              << std::setw(30) << "25 percent jobs cpu heavy" << " | " 
              << std::setw(12) << TOTAL_JOBS << " | " 
              << std::fixed << std::setprecision(3) << stats.throughput_mps << " M/s   " << " | " 
              << std::setw(7) << std::setprecision(2) << stats.p50_lat_us << " us" << " | " 
              << std::setw(7) << stats.p99_lat_us << " us" << "\n";

    scheduler.stop();
    return 0;
}