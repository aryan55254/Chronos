# Chronos — Test Results

This document tracks the validation of the core concurrency primitives in Chronos.
All tests were compiled with strict warnings and validated under **AddressSanitizer (ASAN)**.

---

## 1. Deque Tests (Chase–Lev)

**Status:** ✅ **PASS**

We validated the Lock-Free Work-Stealing Deque under two scenarios:
1.  **Single-Thread Owner:** Verified strict LIFO ordering and capacity checks.
2.  **Concurrent Stealing:** Verified that a thief thread can safely steal tasks while the owner pushes/pops, with zero lost jobs or duplicate executions.

**Run Log:**
* `test_deque_single`: [OK]
* `test_deque_two_thread`: [OK] (1000 jobs executed, ~20% stolen)

![Deque Test Screenshot](public/deque_test.png)

---

## 2. Mailbox Tests (MPSC Spinlock)

**Status:** ✅ **PASS**

We validated the Multi-Producer Single-Consumer (MPSC) Mailbox used for I/O injection:
1.  **FIFO Ordering:** Verified that the API/Reactor preserves execution order.
2.  **High Contention:** Verified 4 concurrent Producer threads feeding 1 Consumer.
    * **Result:** 40,000 jobs processed successfully.
    * **Data Integrity:** 0 jobs lost, 0 corruptions.

**Run Log:**
* `test_mailbox_single`: [OK]
* `test_mailbox_mpsc`: [OK]

![Mailbox Test Screenshot](public/mailbox_test.png)