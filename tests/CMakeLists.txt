# Test configuration for Chronos
cmake_minimum_required(VERSION 3.15)

# Enable testing
enable_testing()

# Download and configure Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Test executables
add_executable(deque_test test_deque.cpp)
target_link_libraries(deque_test PRIVATE chronos_core gtest_main gmock_main)

add_executable(scheduler_test test_scheduler.cpp)
target_link_libraries(scheduler_test PRIVATE chronos_core gtest_main gmock_main)

add_executable(workers_test test_workers.cpp)
target_link_libraries(workers_test PRIVATE chronos_core gtest_main gmock_main)

add_executable(integration_test test_integration.cpp)
target_link_libraries(integration_test PRIVATE chronos_core gtest_main gmock_main)

add_executable(cmake_config_test test_cmake_config.cpp)
target_link_libraries(cmake_config_test PRIVATE gtest_main)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(deque_test)
gtest_discover_tests(scheduler_test)
gtest_discover_tests(workers_test)
gtest_discover_tests(integration_test)
gtest_discover_tests(cmake_config_test)