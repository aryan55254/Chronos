/*
this is basically a worker thread class definition which will be used by the scheduler layer to create a pool of these works to  pop from the mailbox and push pop and execute from the deque ach thread will own each deque and an mailbox
*/
#pragma once

#include <thread>
#include <atomic>
#include "Deque.hpp"
#include "Mailbox.hpp"

class Scheduler; // forward declaration

class Worker
{
public:
    // Unique ID (0 to N-1) for identification and stealing logic
    int id;

    // The system thread running this worker
    std::thread thread;

    // 1. Private LIFO Queue (Fastest, Local)
    // Only this worker pushes/pops bottom. Thieves steal top.
    Deque deque;

    // 2. Public FIFO Queue (Buffered, Thread-Safe)
    // API and Reactor push here via Spinlock. This worker pops wait-free.
    Mailbox mailbox;

    // Pointer to the parent Scheduler (needed to find victims)
    Scheduler *scheduler;

    // Control flag: false = stop looping.
    // alignas(64) isolates this flag to its own cache line to prevent false sharing.
    alignas(64) std::atomic<bool> running;

public:
    // Constructor
    Worker(int id, Scheduler *scheduler);

    // No copying allowed (Threads cannot be copied)
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;

    // Lifecycle
    void start();
    void stop();

    // The Core Loop (The Engine)
    void run();
};