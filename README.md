# Chronos — Implementation Checklist

High-performance asynchronous job scheduler built around a **lock-free work-stealing deque**, **priority scheduling**, and an **epoll-based I/O reactor**.

---

## A) Core: Lock-Free Work-Stealing Deque

### Task 1: Data Structures & Memory Layout

- [ ] Define a `Job` struct with:
  - function pointer or callable storage
  - pointer/reference to promise state (optional)
  - small, trivially movable layout

- [ ] Decide maximum job size (e.g. **64–128 bytes**) to enforce SBO  
- [ ] Design a `TaskNode` (intrusive, reused from pool)  
- [ ] Implement a fixed-capacity circular array for the deque  
- [ ] Choose index type (`uint32_t` or `uint64_t`)  
- [ ] Use **power-of-two capacity** + bitmask for fast modulo  
- [ ] Separate `top` and `bottom` indices  

- [ ] Apply `alignas(64)` to:
  - `top` index
  - `bottom` index
  - frequently written flags

- [ ] Ensure deque memory is allocated **once at startup**  
- [ ] Touch all memory pages during init (avoid page faults later)

---

### Task 2: Lock-Free Deque Logic (Chase–Lev)

- [ ] Implement `push_bottom(Job)`
  - owner thread only
  - write job → store (release) → increment `bottom`

- [ ] Implement `pop_bottom()`
  - owner thread only
  - decrement `bottom`
  - check race with steal
  - handle last-item case carefully

- [ ] Implement `steal_top()`
  - thieves only
  - CAS on `top`
  - acquire semantics

- [ ] Handle empty & single-item edge cases  
- [ ] Verify **no CAS** in owner fast path  
- [ ] Ensure memory ordering correctness  
- [ ] Add debug asserts for invariants:
  - `top <= bottom`
  - capacity bounds

---

### Task 3: Work-Stealing Testing

- [ ] Single-thread push/pop correctness test  
- [ ] Two-thread steal test  
- [ ] Stress test with **N workers**  
- [ ] Validate no duplicate execution  
- [ ] Validate no lost tasks  
- [ ] Run under **TSAN / ASAN**  
- [ ] Add counters for steal frequency  

---

### Task 4: Worker Pool

- [ ] Create `Worker` struct:
  - thread
  - local deques (per priority)
  - thread-local pools

- [ ] Create `WorkerPool` manager  
- [ ] Pin workers to CPU cores (optional but recommended)

- [ ] Worker main loop:
  - pop from highest-priority local deque
  - if empty → steal
  - if still empty → backoff / sleep

- [ ] Implement adaptive backoff:
  - `_mm_pause()`
  - `std::this_thread::yield()`

- [ ] Safe exit condition on shutdown  
- [ ] Join all threads cleanly  

---

## B) Scheduler Engine (Policy Layer)

### Task 5: Priority Queues

- [ ] Decide number of priority levels (e.g. **3**)  
- [ ] Each worker owns:
  - `deque[HIGH]`
  - `deque[MEDIUM]`
  - `deque[LOW]`

- [ ] Push policy:
  - tasks enter local deque when possible

- [ ] Pop policy:
  - always try **HIGH → MED → LOW**

- [ ] Steal policy:
  - steal **HIGH** first

- [ ] Optional: priority aging counters  
- [ ] Avoid global scans  

---

## C) Epoll Reactor (I/O Producer)

### Task 6: Epoll Wrapper

- [ ] Create `EpollReactor` class  
- [ ] Call `epoll_create1(EPOLL_CLOEXEC)`  
- [ ] Preallocate `epoll_event events[MAX_EVENTS]`  
- [ ] Wrap `epoll_ctl` (ADD / MOD / DEL)  
- [ ] Use **non-blocking sockets only**  
- [ ] Decide LT vs ET (**LT first for safety**)  

---

### Task 7: Reactor Loop

- [ ] Run epoll loop in a dedicated thread  
- [ ] Call `epoll_wait()`  
- [ ] For each event:
  - extract fd & flags
  - package into `Job`

- [ ] Submit job into scheduler  
- [ ] Ensure submission path is **lock-free**  
- [ ] No heavy work inside reactor thread  

---

## D) Runtime API (Entry Point)

### Task 8: Promise / Future System

- [ ] Design `FutureState` struct:
  - value storage
  - ready flag
  - synchronization primitive

- [ ] Preallocate pool of `FutureState`  
- [ ] Implement `Promise::set_value`  
- [ ] Implement `Future::get` (blocking or spin+wait)  
- [ ] Ensure single completion  
- [ ] Handle task failure paths  

---

### Task 9: Public API

- [ ] `Chronos::start()`  
- [ ] `Chronos::submit(priority, callable)`  
- [ ] `Chronos::shutdown()`  
- [ ] Reject submissions after shutdown  
- [ ] Drain queues on exit  
- [ ] Resolve or cancel pending futures  

---

## E) Final Stability & Validation

- [ ] No runtime allocations after startup  
- [ ] No worker busy-spin under idle load  
- [ ] Correct shutdown under stress  
- [ ] No task executed twice  
- [ ] No lost tasks  
- [ ] p95 / p99 latency measurements  
- [ ] README with architecture diagram  

---
