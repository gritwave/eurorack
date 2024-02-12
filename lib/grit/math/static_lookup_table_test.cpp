#include "static_lookup_table.hpp"
#include "static_lookup_table_transform.hpp"

#include <etl/cmath.hpp>
#include <etl/numbers.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("grit/math: StaticLookupTable", "", float, double)
{
    using Float = TestType;

    auto lut = grit::StaticLookupTable<Float, 127>{[](auto index) { return static_cast<Float>(index); }};
    REQUIRE(lut[Float(0)] == Catch::Approx(0.0));
    REQUIRE(lut[Float(55)] == Catch::Approx(55.0));
    REQUIRE(lut[Float(126)] == Catch::Approx(126.0));

    REQUIRE(lut.at(Float(100)) == Catch::Approx(100.0));
    REQUIRE(lut.at(Float(125)) == Catch::Approx(125.0));
    REQUIRE(lut.at(Float(126)) == Catch::Approx(126.0));
    REQUIRE(lut.at(Float(127)) == Catch::Approx(126.0));
    REQUIRE(lut.at(Float(128)) == Catch::Approx(126.0));
}

TEMPLATE_TEST_CASE("grit/math: StaticLookupTableTransform", "", float, double)
{
    using Float = TestType;

    auto const func = [](auto val) { return etl::sin(val); };
    auto const min  = Float(-etl::numbers::pi);
    auto const max  = Float(+etl::numbers::pi);

    auto lut = grit::StaticLookupTableTransform<Float, 2047>{func, min, max};
    REQUIRE(lut[min] == Catch::Approx(func(min)).margin(0.001));
    REQUIRE(lut[max] == Catch::Approx(func(max)).margin(0.001));
    REQUIRE(lut[Float(-1)] == Catch::Approx(func(Float(-1))));
    REQUIRE(lut[Float(+1)] == Catch::Approx(func(Float(+1))));

    REQUIRE(lut.at(min) == Catch::Approx(func(min)).margin(0.001));
    REQUIRE(lut.at(max) == Catch::Approx(func(max)).margin(0.001));
    REQUIRE(lut.at(Float(-1)) == Catch::Approx(func(Float(-1))));
    REQUIRE(lut.at(Float(+1)) == Catch::Approx(func(Float(+1))));
}
