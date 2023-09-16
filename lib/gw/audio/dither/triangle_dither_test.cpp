#include "triangle_dither.hpp"

#include <gw/testing/assert.hpp>

#include <random>

template<typename URNG>
[[nodiscard]] static auto test() noexcept -> bool
{
    auto dither = gw::TriangleDither<URNG>{std::random_device{}()};
    for (auto i{0}; i < 100; ++i) {
        assert(0.5 <= dither(1.0F) <= 1.5);
    }

    return true;
}

auto test_triangle_dither() -> bool;

auto test_triangle_dither() -> bool
{
    assert((test<etl::xorshift32>()));
    assert((test<etl::xoshiro128plus>()));
    return true;
}
