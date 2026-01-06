#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include "Workers.hpp"
#include "Job.hpp"

class Scheduler
{

private:
    // thread pool
    size_t num_workers;
    Worker *workers[4];
    std::atomic<size_t> next_worker_index;

public:
    // constructor and destrutor
    Scheduler();
    ~Scheduler();

    void start();
    void stop();
    void Submit(Job job);

    inline size_t get_worker_count() const
    {
        return num_workers;
    }

    inline Worker *get_worker(size_t index)
    {
        // Raw array access - fastest possible lookup
        return workers[index];
    }
};