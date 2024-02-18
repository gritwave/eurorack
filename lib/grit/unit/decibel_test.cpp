#include "decibel.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("unit: toDecibels/fromDecibels", "", float, double)
{
    using Float = TestType;

    auto const infinity = grit::defaultMinusInfinityDb<Float>;

    REQUIRE(grit::toDecibels(Float(0)) == Catch::Approx(infinity));
    REQUIRE(grit::toDecibels(Float(0.00000001)) == Catch::Approx(infinity));
    REQUIRE(grit::fromDecibels(infinity) == Catch::Approx(Float(0)));

    REQUIRE(grit::toDecibels(grit::fromDecibels(Float(0))) == Catch::Approx(Float(0)));
    REQUIRE(grit::toDecibels(grit::fromDecibels(Float(-6))) == Catch::Approx(Float(-6)));
    REQUIRE(grit::toDecibels(grit::fromDecibels(Float(-12))) == Catch::Approx(Float(-12)));
}

TEMPLATE_TEST_CASE("unit: Decibels", "", float, double)
{
    using Float = TestType;

    auto const infinity = grit::defaultMinusInfinityDb<Float>;
    REQUIRE(grit::Decibels<float>::fromGain(Float(0)).value() == Catch::Approx(infinity));

    REQUIRE((grit::Decibels{Float(1)} + grit::Decibels{Float(1)}).value() == Catch::Approx(2));
    REQUIRE((grit::Decibels{Float(1)} - grit::Decibels{Float(1)}).value() == Catch::Approx(0));

    REQUIRE((grit::Decibels{Float(1)} / Float(2)).value() == Catch::Approx(0.5));
    REQUIRE((grit::Decibels{Float(1)} * Float(2)).value() == Catch::Approx(2));
}
