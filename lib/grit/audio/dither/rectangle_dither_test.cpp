#include "rectangle_dither.hpp"

#include <grit/testing/assert.hpp>

#include <random>

template<typename URNG>
[[nodiscard]] static auto test() -> bool
{
    auto dither = grit::RectangleDither<URNG>{std::random_device{}()};
    for (auto i{0}; i < 100; ++i) {
        auto const val = dither(1.0F);
        assert(val >= 0.5);
        assert(val <= 1.5);
    }

    return true;
}

auto testRectangleDither() -> bool;

auto testRectangleDither() -> bool
{
    assert((test<etl::xorshift32>()));
    assert((test<etl::xoshiro128plus>()));
    return true;
}
