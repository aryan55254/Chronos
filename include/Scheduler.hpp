#pragma once

#include <atomic>
#include <memory>
#include <future>         
#include <functional>
#include "Workers.hpp"
#include "Job.hpp"
#include <vector>
#include <thread>
#include <type_traits>

class Scheduler
{

private:
    // thread pool
    size_t num_workers;
    Worker *workers[4];
    std::atomic<size_t> next_worker_index;
    void Submit(Job job);

public:
    // constructor and destrutor
    Scheduler();
    ~Scheduler();

    void start();
    void stop();
    
   //promise/future based job submission
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using ReturnType = typename std::invoke_result<F, Args...>::type;

        auto task_ptr = new std::packaged_task<ReturnType()>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ReturnType> res = task_ptr->get_future();

        auto wrapper_func = [](void* arg) {
            auto* typed_task = static_cast<std::packaged_task<ReturnType()>*>(arg);
            
            (*typed_task)(); 

            delete typed_task; 
        };

        Job job;
        job.func = wrapper_func;
        job.arg  = task_ptr; 

        Submit(job);

        return res;
    }
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