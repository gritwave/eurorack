#include "ipow.hpp"

#include <grit/testing/assert.hpp>

auto test_ipow() -> bool;

auto test_ipow() -> bool
{
    assert(grit::ipow(1, 0) == 1);  // NOLINT
    assert(grit::ipow(1, 1) == 1);  // NOLINT
    assert(grit::ipow(1, 2) == 1);  // NOLINT

    assert(grit::ipow(2, 0) == 1);  // NOLINT
    assert(grit::ipow(2, 1) == 2);  // NOLINT
    assert(grit::ipow(2, 2) == 4);  // NOLINT

    assert(grit::ipow<1>(0) == 1);  // NOLINT
    assert(grit::ipow<1>(1) == 1);  // NOLINT
    assert(grit::ipow<1>(2) == 1);  // NOLINT

    assert(grit::ipow<2>(0) == 1);  // NOLINT
    assert(grit::ipow<2>(1) == 2);  // NOLINT
    assert(grit::ipow<2>(2) == 4);  // NOLINT
    return true;
}
