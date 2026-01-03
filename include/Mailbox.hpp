#pragma once

#include <atomic>
#include "../include/Job.hpp"

/*
this is basically a high performance spin lock MPSC queue,  reactor and main threads can push jobs here but only the workers can pop from it and each worker own one and can pop from it only the stealing doesn't happen in this one this acts kinda like a message box to the deque , the push because it can be touched by to threads uses spin lcok which is blazingly faster than mutexes locks and also than the lock free flag reading way which is also slower than this because it has to read again and again
*/

class Mailbox
{
private:
  /*
  internal ring buffer , here buffer is the actual buffer made up of jobs , capacity is the max amount of elements in the buffer and the mask is way for the ring logic
  */
  Job *buffer;
  size_t capacity;
  size_t mask;
  /* CACHE ISOLATION:
       - producer_lock: fighting happens here.
       - Head: modified by producer.
       - Tail: modified by consumer.
       We separate them so the Worker doesn't get cache invalidations
       just because the API is acquiring the lock.
    */
  alignas(64) std::atomic<size_t> Head;
  alignas(64) std::atomic<size_t> Tail;
  alignas(64) std::atomic_flag producer_lock = ATOMIC_FLAG_INIT;

public:
  /*
     Constructor:
     - Initialize top and bottom to 0
     - Capacity must be power of two
     - Buffer allocated once
     */
  explicit Mailbox(size_t capacity);
  /*
  prevent duplication of ownership of the queue
  */
  Mailbox(const Mailbox &) = delete;
  Mailbox &operator=(const Mailbox &) = delete;

  /*
  Destructor we release buffer aloocations in this
  */
  ~Mailbox();
  /*
  push_bottom:
  - called by the reactor thread or main thread
  - Place job into buffer[bottom]
  - Publish by incrementing bottom
  */
  bool push_head(Job job);

  /*
  pop_bottom:
  - ONLY called by owner thread
  - Remove most recent job
  */
  bool pop_tail(Job &out);
};
