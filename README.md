# Chronos

Chronos is a **low-latency C++ job scheduler** designed for predictable performance, minimal contention, and high-throughput I/O integration.

It employs a **hybrid queuing architecture**:
1. **Work-Stealing Deque (LIFO):** A lock-free ring buffer for thread-local task recursion.
2. **Injection Mailbox (FIFO):** A high-performance MPSC (Multi-Producer Single-Consumer) queue for external task injection from the API.

The project focuses on **correctness, cache locality, and explicit concurrency control**, strictly avoiding dynamic allocation and global contention on the scheduling hot path.

###  Performance Highlights
* **Latency:** **< 5µs** (P50) for lightweight and mixed(1/4th heavy) tasks via spin-wait architecture.
* **Throughput:** **> 1.0 Million ops/sec** on commodity hardware (8 cores) for mixed tasks.
* **Scaling:** **Perfect 4x speedup** on 4 threads for compute-bound workloads.

Check detailed validation and performance benchmarks, please refer to [TestResults.md](TestResults.md).

---

### Architecture Diagram

![ARCHITECTURE](public/architecture.png)

## Design Goals

* **Predictable Latency:** Wait-free execution for workers consuming their own queues.
* **Cache Locality:** Per-thread ownership of queues with `alignas(64)` to prevent false sharing.
* **MPSC/SPSC Hybrid:** Optimized injection paths (Spinlock) vs. execution paths (Lock-Free/Wait-Free).
* **No Hot-Path Allocations:** All queue memory is pre-allocated at startup.
* **Correctness:** Validated under AddressSanitizer (ASAN) and stress tests.

---

## Performance Architecture

Chronos achieves **sub-5µs latency** by utilizing a **Latency-Critical Design**:

* **Spin-Wait Spinning:** Worker threads utilize `_mm_pause()` instructions instead of OS-level sleeping . This eliminates the overhead of context switching and kernel wakeups.
* **Hot-Cache Execution:** By keeping threads active, the instruction cache remains hot, ensuring that new tasks are picked up immediately upon submission.
* **Trade-off:** This design deliberately targets **Maximum Throughput** and **Minimum Latency** at the cost of higher CPU utilization.

The Scheduling Runtime is fully functional, capable of executing heavy parallel workloads with near-perfect linear scaling and handling adversarial submission patterns via randomized load balancing.

#### 1. The Scheduler (Orchestrator)
* **Injection Policy:** **Randomized Distribution**.
    * Tasks are pushed to a random worker's mailbox.
    * **Why:** Replaces deterministic Round-Robin to prevent "convoy effects" (overloading a single thread) during patterned task injection sequences.
* **Submission Backoff:** **Adaptive Backoff**.
    * If a mailbox is contended or full, the submission utilizes a `Spin -> Yield` strategy to manage overflow without blocking the main thread aggressively.
* **Topology:** Maps 1 Worker per Thread.
* **I/O:** Uses std::future/promise based I/O and allows async input of jobs.

#### 2. Lock-Free Work-Stealing Deque (Chase–Lev)
* **Role:** Stores thread-local sub-tasks (recursion).
* **Stealing Policy:** **Randomized Victim Selection**.
    * When a worker is empty, it attempts to steal from a random peer.
* **Semantics:** LIFO (Last-In-First-Out) for the owner; FIFO (First-In-First-Out) for thieves.
* **Synchronization:**
  * Owner: **Wait-Free** (atomic loads/stores) for push/pop.
  * Thieves: **Lock-Free** (CAS loop) to steal tasks.
* **Conflict Resolution:** Handles the "single-item race" using `compare_exchange_strong`.

#### 3. MPSC Injection Mailbox
* **Role:** Buffers tasks coming from external sources.
* **Semantics:** Strict FIFO (First-In-First-Out).
* **Synchronization:**
  * **Push (Producers):** Uses a lightweight `atomic_flag` **Spinlock**.
  * **Pop (Worker):** **Wait-Free**.
* **Safety:** Fully immune to "Phantom Reads" via strict `acquire`/`release` ordering.
---

## Build & Development

Chronos targets **C++20** and uses CMake.

### Build Instructions

```bash
mkdir build && cd build

# Release mode with Native optimizations is critical for benchmarks
cmake -DCMAKE_BUILD_TYPE=Release -DCHRONOS_NATIVE=ON ..
make -j

```

### Running Tests & Benchmarks

**1. Unit Tests (Data Structures)**

```bash
./test_deque_single     # Validate Deque logic
./test_mailbox_mpsc     # Validate Mailbox logic

```

**2. Performance Benchmarks**

```bash
./benchmark_easy   # 10 Million Jobs (Throughput + Low Latency Check)
./benchmark_mixed  # 10 Million Jobs (Load Balancing + Heavy Math)

```

**3. Demo Application**

```bash
./chronos_demo     # Visual proof of parallelism (Main thread non-blocking)

```