# Chronos

Chronos is a **low-latency C++ job scheduler** designed for predictable performance and minimal contention.
It is built around a **lock-free, per-thread work-stealing deque** (Chase–Lev) and is intended to support **priority-aware scheduling**, **epoll-based I/O integration**, and a **future/promise-based execution API**.

The project focuses on **correctness, cache locality, and explicit concurrency control**, avoiding dynamic allocation and global contention on the scheduling hot path.

---

## Design Goals

* Lock-free task scheduling with predictable latency
* Per-thread ownership to minimize contention
* Cache-friendly memory layout
* No allocations on the scheduling hot path
* Clear separation between core execution, scheduling policy, and API
* Correctness validated under stress and sanitizers

---

## Current Status

Chronos is under active development.
The **core execution primitive** — the lock-free work-stealing deque — is **fully implemented and validated**.

### Implemented

#### Lock-Free Work-Stealing Deque (Chase–Lev)

* Per-thread deque with strict ownership semantics
* Owner fast path is CAS-free
* Thieves steal from the opposite end using CAS
* Correct handling of:

  * empty deque
  * single-item race
  * concurrent steals
* Power-of-two ring buffer with bitmask indexing
* `alignas(64)` padding to avoid false sharing
* Memory allocated once at startup
* No dynamic allocation on the scheduling hot path
* Correct acquire / release / acq_rel memory ordering

#### Testing & Validation

* Single-thread push/pop correctness tests
* Two-thread steal correctness tests
* Validation for:

  * no duplicate execution
  * no lost tasks
* Clean runs under:

  * AddressSanitizer (ASAN)

The deque implementation  will not be modified except for micro-optimizations.

---

## Planned Architecture

Chronos is structured in layers, built incrementally on top of the deque.

### Worker Pool (Next)

* Fixed set of worker threads
* One or more deques per worker
* Work-stealing execution loop
* Adaptive backoff (pause → yield)
* Clean shutdown and thread joining

### Scheduler / Policy Layer

* Priority-aware scheduling (e.g. high / medium / low)
* Per-worker priority deques
* Priority-first pop and steal policy
* Optional aging to prevent starvation
* No global scheduling queues

### Epoll-Based I/O Reactor

* Dedicated epoll thread
* Non-blocking I/O only
* Event-to-job translation
* Lock-free submission into the scheduler
* Minimal work inside the reactor thread

### Runtime API

* Promise / future-based task submission
* Explicit lifecycle management (`start`, `submit`, `shutdown`)
* Rejection of submissions after shutdown
* Graceful draining or cancellation of pending work

---

## Non-Goals

* General-purpose thread pool abstraction
* Hidden blocking or implicit synchronization
* Dynamic task graphs or heavy runtime reflection
* “Fire-and-forget” task semantics without ownership guarantees

Chronos prioritizes **explicitness and predictability** over convenience.

---

## Build & Development

Chronos uses CMake and targets modern C++ (C++20).

The project is structured into:

* `src/` and `include/` for core implementation
* `tests/` mirroring the architecture for isolated testing

Sanitizers (ASAN / TSAN) are supported and used during development to validate correctness under concurrency.

---

## Roadmap

1. Worker pool implementation
2. Priority-aware scheduling policy
3. Epoll-based I/O reactor
4. Promise / future API
5. End-to-end latency measurements
6. Documentation and architecture diagrams

---

## License

MIT License.

---
