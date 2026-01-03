# Chronos

Chronos is a **low-latency C++ job scheduler** designed for predictable performance, minimal contention, and high-throughput I/O integration.

It employs a **hybrid queuing architecture**:
1. **Work-Stealing Deque (LIFO):** A lock-free ring buffer for thread-local task recursion.
2. **Injection Mailbox (FIFO):** A high-performance MPSC (Multi-Producer Single-Consumer) queue for external task injection from the API or I/O Reactor.

The project focuses on **correctness, cache locality, and explicit concurrency control**, strictly avoiding dynamic allocation and global contention on the scheduling hot path.

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

## Current Status

Chronos is under active development. The **core data structures** are fully implemented and validated.

### Implemented Components

#### 1. Lock-Free Work-Stealing Deque (Chase–Lev)
* **Role:** Stores thread-local sub-tasks (recursion).
* **Semantics:** LIFO (Last-In-First-Out) for the owner; FIFO (First-In-First-Out) for thieves.
* **Synchronization:** * Owner: **Wait-Free** (atomic loads/stores) for push/pop.
  * Thieves: **Lock-Free** (CAS loop) to steal tasks.
* **Conflict Resolution:** Handles the "single-item race" (Owner popping vs Thief stealing) using `compare_exchange_strong`.

#### 2. MPSC Injection Mailbox
* **Role:** Buffers tasks coming from the Main Thread (API) or Epoll Reactor.
* **Semantics:** Strict FIFO (First-In-First-Out).
* **Synchronization:**
  * **Push (Producers):** Uses a lightweight `atomic_flag` **Spinlock**. This allows multiple external threads (Reactor + API) to safely inject tasks without corrupting the index.
  * **Pop (Worker):** **Wait-Free**. The worker never touches the lock, ensuring external contention never stalls the execution engine.
* **Safety:** Fully immune to "Phantom Reads" via strict `acquire`/`release` ordering.

### Validation
* **Correctness Tests:** Single-threaded and Multi-threaded (Stealing/Contention) tests passing.
* **Sanitizers:** Clean runs under ASAN/TSAN.
* **Memory Safety:** No leaks; RAII-compliant buffer management.

---

## Planned Architecture

Chronos is structured in layers:

### 1. The Worker (Execution Engine)
* Owns 1 `Deque` (Private) and 1 `Mailbox` (Public).
* **Priority Rule:** 1. Process Local Deque (Hot Cache).
  2. Process Mailbox (External Input).
  3. Steal from others (Load Balancing).

### 2. The Epoll Reactor (I/O Layer)
* A dedicated thread running `epoll_wait`.
* Translates socket events into `Job` structs.
* Pushes jobs directly into a specific Worker's `Mailbox` (using the MPSC path).

### 3. The Scheduler (Orchestrator)
* Manages the lifecycle of workers.
* Provides the "victim finding" logic for work stealing.

---

## Build & Development

Chronos targets **C++20** and uses CMake.

### Build Instructions
```bash
mkdir build && cd build
cmake .. -DENABLE_ASAN=ON
make
```
### Running Tests

```
./test_deque_single
./test_deque_two_thread
./test_mailbox_single
./test_mailbox_mpsc
```