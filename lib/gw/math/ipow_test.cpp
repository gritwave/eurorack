#include "ipow.hpp"

#include <gw/testing/assert.hpp>

auto test_ipow() -> bool;

auto test_ipow() -> bool
{
    assert(gw::ipow(1, 0) == 1);
    assert(gw::ipow(1, 1) == 1);
    assert(gw::ipow(1, 2) == 1);

    assert(gw::ipow(2, 0) == 1);
    assert(gw::ipow(2, 1) == 2);
    assert(gw::ipow(2, 2) == 4);

    assert(gw::ipow<1>(0) == 1);
    assert(gw::ipow<1>(1) == 1);
    assert(gw::ipow<1>(2) == 1);

    assert(gw::ipow<2>(0) == 1);
    assert(gw::ipow<2>(1) == 2);
    assert(gw::ipow<2>(2) == 4);
    return true;
}
