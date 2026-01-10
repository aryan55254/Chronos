#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include "../include/Scheduler.hpp"

// A task that takes time but yields a value
int heavy_calc(int id) {
    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return id * id;
}

int main() {
    std::cout << "[Demo] Starting Chronos Application...\n";
    
    Scheduler scheduler;
    scheduler.start();

    const int JOBS = 8;
    std::cout << "[Demo] Submitting " << JOBS << " heavy tasks...\n";

    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::future<int>> results;

    //  Submit Jobs
    for(int i = 1; i <= JOBS; i++) {
        results.push_back(scheduler.submit([i] {
            return heavy_calc(i);
        }));
    }

    std::cout << "[Demo] Tasks submitted. Main thread is free!\n";
    
    //  Wait for Results
    for(int i = 0; i < JOBS; i++) {
        int val = results[i].get();
        std::cout << "  -> Task " << i+1 << " Result: " << val << "\n";
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    std::cout << "[Demo] Total Time: " << elapsed.count() << " ms\n";
    
    if(elapsed.count() < 401) std::cout << "[PASS] Parallel Speedup Detected!\n";
    else std::cout << "[WARN] Seemed sequential?\n";

    scheduler.stop();
    return 0;
}
//instead of the .get which we do at end because it is blocking we could do polling by .wait_for which would check for result at time intervals , you won't have to wait for results at end neither have to block the main thread