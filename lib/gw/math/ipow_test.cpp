#include "ipow.hpp"

#undef NDEBUG
#include <cassert>

[[nodiscard]] static constexpr auto test() -> bool
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

auto main() -> int
{
    assert(test());
    static_assert(test());
    return 0;
}