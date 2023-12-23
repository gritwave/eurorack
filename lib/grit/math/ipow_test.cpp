#include "ipow.hpp"

#include <grit/testing/assert.hpp>

auto test_ipow() -> bool;

auto test_ipow() -> bool
{
    assert(grit::ipow(1, 0) == 1);
    assert(grit::ipow(1, 1) == 1);
    assert(grit::ipow(1, 2) == 1);

    assert(grit::ipow(2, 0) == 1);
    assert(grit::ipow(2, 1) == 2);
    assert(grit::ipow(2, 2) == 4);

    assert(grit::ipow<1>(0) == 1);
    assert(grit::ipow<1>(1) == 1);
    assert(grit::ipow<1>(2) == 1);

    assert(grit::ipow<2>(0) == 1);
    assert(grit::ipow<2>(1) == 2);
    assert(grit::ipow<2>(2) == 4);
    return true;
}
