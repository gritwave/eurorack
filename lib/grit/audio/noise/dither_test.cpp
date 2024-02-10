#include "dither.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <random>

TEMPLATE_PRODUCT_TEST_CASE(
    "grit/audio/noise: Dither",
    "",
    (grit::NoDither, grit::RectangleDither, grit::TriangleDither),
    (etl::xorshift32, etl::xoshiro128plus, std::mt19937)
)
{
    using Dither = TestType;

    auto dither = Dither{std::random_device{}()};
    for (auto i{0}; i < 100; ++i) {
        auto const val = dither(1.0F);
        REQUIRE(val >= 0.5);
        REQUIRE(val <= 1.5);
    }
}
