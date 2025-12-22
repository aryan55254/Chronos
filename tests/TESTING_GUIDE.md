# Chronos Testing Guide

## Introduction

This guide provides detailed information on testing the Chronos job scheduler, including test philosophy, implementation guidelines, and troubleshooting tips.

## Test Philosophy

The Chronos test suite is designed with the following principles:

1. **Correctness First**: Lock-free algorithms are notoriously difficult to get right. Tests must thoroughly validate correctness before considering performance.

2. **Test-Driven Development**: Tests are written before implementation to guide design and ensure comprehensive coverage.

3. **Incremental Implementation**: Tests are marked with `GTEST_SKIP()` and enabled as features are implemented.

4. **Multi-Level Testing**: Tests span from unit tests (individual components) to integration tests (full system).

5. **Sanitizer-Friendly**: All tests must pass under AddressSanitizer and ThreadSanitizer.

## Critical Test Areas

### 1. Lock-Free Deque (Chase-Lev Algorithm)

The work-stealing deque is the foundation of the scheduler. Critical areas:

#### Single-Item Race Condition
This is the most subtle bug in Chase-Lev deques - when deque has exactly 1 item, both owner and thief may try to take it. Only ONE should succeed!

#### Memory Ordering
push_bottom must use release semantics, steal_top must use acquire semantics.

#### Capacity and Wrapping
Power-of-two capacity with bitmask for fast modulo operations.

### 2. Priority Scheduling

Priority scheduling must prevent starvation while respecting priorities.

### 3. Work Stealing

Work stealing must balance load without excessive overhead.

## Additional Resources

- Lock-Free Programming
- Chase-Lev Work Stealing Paper
- C++ Memory Model Documentation
- Google Test Primer
- Sanitizer Documentation