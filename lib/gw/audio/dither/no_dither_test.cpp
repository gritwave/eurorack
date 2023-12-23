#include "no_dither.hpp"

#include <gw/testing/approx.hpp>
#include <gw/testing/assert.hpp>

#include <random>

template<typename URNG>
[[nodiscard]] static auto test() noexcept -> bool
{
    auto dither = gw::NoDither<URNG>{std::random_device{}()};
    for (auto i{0}; i < 100; ++i) {
        auto const val = dither(1.0F);
        assert(val >= 0.5);
        assert(val <= 1.5);
        assert(gw::approx(val, 1.0F));
    }

    return true;
}

auto test_no_dither() -> bool;

auto test_no_dither() -> bool
{
    assert((test<etl::xorshift32>()));
    assert((test<etl::xoshiro128plus>()));
    return true;
}
