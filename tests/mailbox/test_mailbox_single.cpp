#include "../../include/Mailbox.hpp"
#include <cassert>
#include <iostream>

static void noop(void *) {}

int main()
{
    // Create a mailbox with capacity 4 (small for testing overflow)
    Mailbox mb(4);

    Job job1{noop, (void *)1};
    Job job2{noop, (void *)2};
    Job job3{noop, (void *)3};
    Job job4{noop, (void *)4};
    Job job5{noop, (void *)5};

    // 1. Fill it up
    assert(mb.push_head(job1));
    assert(mb.push_head(job2));
    assert(mb.push_head(job3));
    assert(mb.push_head(job4));

    // 2. Test Overflow (Should fail)
    assert(!mb.push_head(job5));

    // 3. Pop and Verify FIFO Order (1 -> 2 -> 3 -> 4)
    Job out;

    assert(mb.pop_tail(out));
    assert(out.arg == (void *)1);

    assert(mb.pop_tail(out));
    assert(out.arg == (void *)2);

    // 4. Push again after making space
    assert(mb.push_head(job5));

    // 5. Drain the rest
    assert(mb.pop_tail(out)); // 3
    assert(out.arg == (void *)3);

    assert(mb.pop_tail(out)); // 4
    assert(out.arg == (void *)4);

    assert(mb.pop_tail(out)); // 5
    assert(out.arg == (void *)5);

    // 6. Test Empty
    assert(!mb.pop_tail(out));

    std::cout << "[OK] Mailbox single-thread FIFO test\n";
    return 0;
}