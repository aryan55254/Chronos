#pragma once

#include <atomic>
#include <cstddef>

/*
this basically contains a function pointer and an opaque argument
*/

struct Job
{
  void (*func)(void *);
  void *arg;
};
