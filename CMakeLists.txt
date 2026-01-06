cmake_minimum_required(VERSION 3.15)

project(Chronos VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Dependencies

find_package(Threads REQUIRED)

# Sanitizer options 

option(ENABLE_ASAN "Enable AddressSanitizer (Memory safety)" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer (Race conditions)" OFF)

# This variable holds sanitizer flags for the entire build
set(SANITIZER_FLAGS "")

if(ENABLE_ASAN)
    message(STATUS "Build Mode: AddressSanitizer Enabled")
    set(SANITIZER_FLAGS
        -fsanitize=address
        -fno-omit-frame-pointer
    )
elseif(ENABLE_TSAN)
    message(STATUS "Build Mode: ThreadSanitizer Enabled")
    set(SANITIZER_FLAGS -fsanitize=thread)
endif()

# Helper function to apply sanitizers CONSISTENTLY
function(apply_sanitizers target)
    if(SANITIZER_FLAGS)
        target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${target} PRIVATE ${SANITIZER_FLAGS})
    endif()
endfunction()

# Core library
add_library(chronos_core
    src/Deque.cpp
    src/Workers.cpp
    src/Scheduler.cpp
    src/Mailbox.cpp
)
target_include_directories(chronos_core PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(chronos_core PRIVATE Threads::Threads)

apply_sanitizers(chronos_core)

# Main runner
add_executable(chronos_runner
    API/main.cpp
)

target_link_libraries(chronos_runner PRIVATE chronos_core)

apply_sanitizers(chronos_runner)

# Compiler warnings & build-type flags

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(chronos_core PRIVATE
        -Wall
        -Wextra
        -Werror
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Release>:-O3>
    )
endif()

option(CHRONOS_NATIVE "Enable native CPU optimizations" OFF)
if(CHRONOS_NATIVE)
    target_compile_options(chronos_core PRIVATE -march=native)
endif()

# Deque tests

add_executable(test_deque_single
    tests/deque/test_deque_single.cpp
)

target_link_libraries(test_deque_single PRIVATE chronos_core Threads::Threads)
apply_sanitizers(test_deque_single)

add_executable(test_deque_two_thread
    tests/deque/test_deque_two_thread.cpp
)

target_link_libraries(test_deque_two_thread PRIVATE chronos_core Threads::Threads)
apply_sanitizers(test_deque_two_thread)

# Mailbox tests

add_executable(test_mailbox_single tests/mailbox/test_mailbox_single.cpp)
target_link_libraries(test_mailbox_single PRIVATE chronos_core Threads::Threads)
apply_sanitizers(test_mailbox_single)

add_executable(test_mailbox_mpsc tests/mailbox/test_mailbox_mpsc.cpp)
target_link_libraries(test_mailbox_mpsc PRIVATE chronos_core Threads::Threads)
apply_sanitizers(test_mailbox_mpsc)

# Compute Layer test

add_executable(test_compute_layer tests/compute_layer/compute_test.cpp)
target_link_libraries(test_compute_layer PRIVATE chronos_core Threads::Threads)
apply_sanitizers(test_compute_layer)

add_executable(test_compute_heavy tests/compute_layer/test_heavy.cpp)
target_link_libraries(test_compute_heavy PRIVATE chronos_core Threads::Threads)
apply_sanitizers(test_compute_heavy)

add_executable(test_compute_mix tests/compute_layer/test_mix.cpp)
target_link_libraries(test_compute_mix PRIVATE chronos_core Threads::Threads)
apply_sanitizers(test_compute_mix)
