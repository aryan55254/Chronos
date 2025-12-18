# minimum versions of cmake and langauge and projects intiiazation 
cmake_minimum_required(VERSION 3.15)

project(Chronos VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# runner directory built , core sourcecode directories bundled and libraries core will used attached 
find_package(Threads REQUIRED)

add_library(chronos_core STATIC
    src/Deque.cpp
    src/Scheduler.cpp
    src/Workers.cpp
)

target_include_directories(chronos_core PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(chronos_core PRIVATE Threads::Threads)

add_executable(chronos_runner API/main.cpp)

# sanitizers to check for thread and memory safety bugs intiazed and added to runner and core 

option(ENABLE_ASAN "Enable AddressSanitizer (Memory safety)" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer (Thread safety)" OFF)

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "You cannot enable ASan and TSan simultaneously! Pick one.")
endif()

if(ENABLE_ASAN)
    message(STATUS "Build Mode: AddressSanitizer Enabled")
    target_compile_options(chronos_core PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(chronos_core PRIVATE -fsanitize=address)

    target_compile_options(chronos_runner PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(chronos_runner PRIVATE -fsanitize=address)
endif()

if(ENABLE_TSAN)
    target_compile_options(chronos_runner PRIVATE -fsanitize=thread)
    target_link_options(chronos_runner PRIVATE -fsanitize=thread)

    target_compile_options(chronos_core PRIVATE -fsanitize=thread)
    target_link_options(chronos_core PRIVATE -fsanitize=thread)
endif()

# core and runner linked
target_link_libraries(chronos_runner PRIVATE chronos_core)

# added compiler flags for better testing acc to different scenarious : release , debugging and native 
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(chronos_core PRIVATE
        -Wall
        -Wextra
        -Werror
    )

    target_compile_options(chronos_core PRIVATE
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Release>:-O3>
    )
endif()

option(CHRONOS_NATIVE "Enable native CPU optimizations" OFF)
if(CHRONOS_NATIVE)
    target_compile_options(chronos_core PRIVATE -march=native)
endif()
