#include "static_lookup_table.hpp"

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
