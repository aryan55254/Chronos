# Chronos Test Suite

This directory contains comprehensive unit and integration tests for the Chronos job scheduler.

## Overview

The test suite is designed to validate all aspects of the Chronos system, from low-level lock-free data structures to high-level API functionality.

## Test Organization

### test_deque.cpp (40+ tests)
Tests for the lock-free work-stealing deque implementation (Chase-Lev algorithm):
- Basic operations (push, pop, steal)
- Single-item edge cases (critical for correctness)
- FIFO/LIFO ordering
- Capacity and circular wrapping
- Memory ordering and atomics
- Concurrency and work stealing
- Stress tests with multiple workers
- Cache line alignment and false sharing
- Thread safety (TSAN/ASAN)

### test_scheduler.cpp (50+ tests)
Tests for the priority-based job scheduler:
- Priority queue management (HIGH, MEDIUM, LOW)
- Priority ordering under load
- Work stealing between workers
- Worker affinity and load balancing
- Adaptive backoff strategies
- Startup and shutdown
- CPU pinning
- Priority aging (anti-starvation)
- Performance and scalability
- Error handling

### test_workers.cpp (65+ tests)
Tests for the worker pool and thread management:
- Worker initialization and lifecycle
- Worker main loop logic
- CPU core pinning
- Adaptive backoff (pause, yield, sleep)
- Work stealing behavior
- Shutdown and cleanup
- Thread-local storage
- Worker identification
- Performance characteristics
- Statistics and monitoring

### test_integration.cpp (60+ tests)
End-to-end integration tests:
- Complete job submission and execution flow
- Promise/Future system
- Public API (Chronos::start, submit, shutdown)
- Work stealing across the system
- Priority system integration
- Memory management
- Performance benchmarks
- Real-world patterns (map-reduce, pipelines, etc.)
- Error handling
- Statistics aggregation

### test_cmake_config.cpp (30+ tests)
Build configuration validation:
- C++20 standard enforcement
- Threading support
- Compiler warnings and flags
- Sanitizer configuration
- C++20 feature availability
- Platform detection
- Atomic operations
- Compiler and architecture detection

## Building and Running Tests

### Build with Tests

```bash
# Create build directory
mkdir -p build && cd build

# Configure with tests enabled
cmake -DCHRONOS_BUILD_TESTS=ON ..

# Build
cmake --build .

# Run all tests
ctest --output-on-failure

# Or run individual test suites
./tests/deque_test
./tests/scheduler_test
./tests/workers_test
./tests/integration_test
./tests/cmake_config_test
```

### Build with Sanitizers

#### AddressSanitizer (Memory Safety)
```bash
cmake -DENABLE_ASAN=ON -DCHRONOS_BUILD_TESTS=ON ..
cmake --build .
ctest --output-on-failure
```

#### ThreadSanitizer (Thread Safety)
```bash
cmake -DENABLE_TSAN=ON -DCHRONOS_BUILD_TESTS=ON ..
cmake --build .
ctest --output-on-failure
```

**Note:** ASan and TSan cannot be enabled simultaneously.

### Debug vs Release Builds

```bash
# Debug build (with debug symbols)
cmake -DCMAKE_BUILD_TYPE=Debug -DCHRONOS_BUILD_TESTS=ON ..

# Release build (with optimizations)
cmake -DCMAKE_BUILD_TYPE=Release -DCHRONOS_BUILD_TESTS=ON ..
```

## Test Development Guidelines

### Current State
All tests are currently marked with `GTEST_SKIP()` as the implementation is not yet complete. As you implement each component:

1. Remove the `GTEST_SKIP()` line
2. Uncomment the `#include` directives for the relevant headers
3. Implement the test logic
4. Verify the test passes

### Adding New Tests

When adding new functionality, follow these guidelines:

1. **Test Naming**: Use descriptive names that explain what is being tested
   - Good: `TEST_F(DequeTest, PopBottomFromEmptyReturnsNullopt)`
   - Bad: `TEST_F(DequeTest, Test1)`

2. **Test Structure**: Follow Arrange-Act-Assert pattern
   ```cpp
   TEST_F(ComponentTest, DescriptiveName) {
       // Arrange: Set up test conditions
       chronos::Deque deque(1024);
       
       // Act: Perform the operation
       auto result = deque.pop_bottom();
       
       // Assert: Verify the outcome
       EXPECT_FALSE(result.has_value());
   }
   ```

3. **Test Categories**: Organize tests into logical groups
   - Basic functionality
   - Edge cases
   - Error handling
   - Concurrency
   - Performance
   - Integration

4. **Concurrency Tests**: For multi-threaded tests
   - Use thread barriers for coordination
   - Collect results atomically
   - Verify both correctness and no data races
   - Run under TSAN

5. **Performance Tests**: For benchmarking
   - Use steady_clock for timing
   - Run multiple iterations
   - Report p50, p95, p99 latencies
   - Compare against baselines

### Mock Usage

Use Google Mock for complex dependencies:
```cpp
class MockScheduler : public chronos::Scheduler {
public:
    MOCK_METHOD(void, submit, (Job job, Priority priority), (override));
    MOCK_METHOD(void, shutdown, (), (override));
};
```

## Testing Best Practices

### 1. Lock-Free Correctness
For lock-free data structures like the deque:
- Test single-item edge cases extensively (Chase-Lev critical section)
- Verify no duplicate executions
- Verify no lost tasks
- Run under ThreadSanitizer

### 2. Memory Safety
- Run under AddressSanitizer
- Check for memory leaks
- Verify no buffer overflows
- Test object lifetime management

### 3. Concurrency
- Test with various worker counts (1, 2, 4, 8, 16, 32)
- High contention scenarios
- Race condition testing
- Proper synchronization verification

### 4. Performance
- Measure latency (p50, p95, p99)
- Measure throughput (jobs/second)
- CPU utilization under idle load
- Scalability with worker count

### 5. Error Handling
- Invalid input handling
- Exception propagation
- Graceful degradation
- Resource cleanup on errors

## Continuous Integration

The test suite is designed to run in CI/CD pipelines:

```bash
# CI script example
#!/bin/bash
set -e

# Build with ASan
cmake -DENABLE_ASAN=ON -DCHRONOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
ctest --output-on-failure

# Clean and rebuild with TSan
rm -rf *
cmake -DENABLE_TSAN=ON -DCHRONOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
ctest --output-on-failure

# Release build performance tests
rm -rf *
cmake -DCHRONOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
ctest --output-on-failure
```

## Test Coverage

To generate test coverage reports (requires gcov/lcov):

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCHRONOS_BUILD_TESTS=ON \
      -DCMAKE_CXX_FLAGS="--coverage" ..
cmake --build .
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Known Issues and TODOs

- [ ] All tests currently skipped pending implementation
- [ ] Performance benchmarks need baseline establishment
- [ ] Epoll reactor tests not yet implemented
- [ ] NUMA-aware tests need platform-specific handling
- [ ] Coverage reporting needs integration

## Contributing

When adding new features:
1. Write tests first (TDD approach)
2. Ensure all tests pass locally
3. Run with both ASan and TSan
4. Update this README if adding new test files
5. Document any platform-specific test requirements

## Resources

- [Google Test Documentation](https://google.github.io/googletest/)
- [Google Mock Documentation](https://google.github.io/googletest/gmock_for_dummies.html)
- [Chase-Lev Deque Paper](https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf)
- [C++ Memory Model](https://en.cppreference.com/w/cpp/atomic/memory_order)
- [ThreadSanitizer Manual](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual)