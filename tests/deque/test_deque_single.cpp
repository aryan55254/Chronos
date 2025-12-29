#include "../../include/Deque.hpp"
#include <cassert>
#include <iostream>

static void noop(void *) {}

int main()
{
    Deque dq(1024);

    Job job{noop, nullptr};

    // Push
    for (int i = 0; i < 1000; ++i)
    {
        bool ok = dq.push_bottom(job);
        assert(ok);
    }

    // Pop
    Job out;
    for (int i = 0; i < 1000; ++i)
    {
        bool ok = dq.pop_bottom(out);
        assert(ok);
    }

    // Empty
    bool ok = dq.pop_bottom(out);
    assert(!ok);

    std::cout << "[OK] single-thread push/pop\n";
    return 0;
}