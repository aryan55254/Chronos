Chronos Implementation Checklist

a) lock free queue : 

Task 1: Data Structures and Memory Layout

[ ] Define a small, Value-Semantics struct Job to hold function pointers and minimal arguments.

[ ] Create the internal Node structure within the queue.

[ ] False Sharing Mitigation: Apply alignas(64) padding to the head and tail atomic indices in MPMCQueue.hpp.

[ ] Finalize the MPMCQueue.hpp structure with the fixed-size std::vector (Ring Buffer) and the bitwise mask for rapid indexing.

Task 2: Lock-Free Logic & Testing

[ ] Implement the Producer logic: push(item). Use the Compare-And-Swap (CAS) loop to atomically claim the next slot at the tail index.

[ ] Implement the Consumer logic: pop_batch(output, max_count). Use a single CAS on the head index to claim a batch of jobs (Adaptive Batching).

[ ] Verification: Write a simple main.cpp test harness that spawns threads to push and pop. Confirm no crashes and correct job order.

Task 3: Worker Pool Setup

[ ] Create the WorkerPool class to manage a fixed number of consumer threads (ideally matching CPU cores).

[ ] Implement Thread RAII: Ensure threads are properly joined/detached in the pool's destructor to prevent resource leaks.

[ ] Implement the Worker Loop: Each thread runs a continuous cycle, calling queue.pop_batch() and executing the jobs it retrieves.

b)  scheduler engine : 

Task 4: Epoll Wrapper

[ ] Create the EpollWrapper class to encapsulate Linux system calls (epoll_create, epoll_ctl, epoll_wait).

[ ] Configuration: Configure the Epoll file descriptor set to use Edge-Triggered (ET) mode for the highest efficiency and performance.

Task 5: The Main Event Loop (The Producer)

[ ] Implement the Scheduler's run() method (the main thread).

[ ] Use epoll_wait() for the Blocking Wait—this thread sleeps until I/O events arrive.

[ ] Event-to-Job Translation: When events return, read them (e.g., new data on a socket) and package the necessary context (FD, event type) into a Job struct.

[ ] Call queue.push(job) to submit the task to the waiting worker threads.

Task 6: Final Integration & Stability

[ ] Connection Check: Ensure the Epoll Producer is pushing to the exact same MPMCQueue instance the Worker Pool is consuming from.

[ ] Stress Test: Run the integrated system under high mock load (simulated connections).

[ ] Confirm low-latency throughput.

[ ] Verify proper backpressure handling if the queue fills up.

[ ] Cleanup: Ensure all system resources are properly closed (epoll_close, socket file descriptors) and threads are terminated cleanly on exit.
