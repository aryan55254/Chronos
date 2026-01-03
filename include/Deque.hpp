#pragma once

#include <atomic>
#include "../include/Job.hpp"

/*
so what we are implementing here is a work stealing per thread ring buffer deque
each task queue is owned by a single worker thread , owner thread can push pop and process work the other threads can steal the job to process it only when they are idle , this makes sure that cache locality is maximised , the workstealing uses atomic CAS(compare and swap) but owner thread doesn't (only in a edge case it does) so this also adds a performance boost compared to a global mpmc queue , we prevent false sharing here as well by cache line padding for top and bottom by this contention is minimized and also all the operations are completely lock free and use atomics , owner uses stack like LIFO by poping from bottom in the queue and thieves do opposite and pop from top
*/

class Deque
{
private:
  /*
    internal ring buffer , here buffer is the actual buffer made up of jobs , capacity is the max amount of elements in the buffer and the mask is way for the ring logic
    */
  Job *buffer;
  size_t capacity;
  size_t mask;
  /*
    Indices:
  - top: accessed by thieves (atomic)
  - bottom: accessed by owner (atomic)
  - Pad to avoid false sharing
  */
  alignas(64) std::atomic<size_t> top;
  alignas(64) std::atomic<size_t> bottom;

public:
  /*
     Constructor:
     - Initialize top and bottom to 0
     - Capacity must be power of two
     - Buffer allocated once
     */
  explicit Deque(size_t capacity);
  /*
  prevent duplication of ownership of the queue
  */
  Deque(const Deque &) = delete;
  Deque &operator=(const Deque &) = delete;

  /*
  Destructor we release buffer aloocations in this
  */
  ~Deque();
  /*
  push_bottom:
  - ONLY called by owner thread
  - Place job into buffer[bottom]
  - Publish by incrementing bottom
  */
  bool push_bottom(Job job);

  /*
  pop_bottom:
  - ONLY called by owner thread
  - Remove most recent job
  - edge case when only one job remains and both thief and owner try to pop it then we use cas here to make sure it is handled well
  */
  bool pop_bottom(Job &out);

  /*
  steal_top:
  - Called by other threads
  - CAS on top index
  - Return false if empty or CAS fails
  */
  bool steal_top(Job &out);
};
