#include "gain_computer.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEMPLATE_PRODUCT_TEST_CASE(
    "audio/dynamic: HardKnee",
    "",
    (grit::HardKneeGainComputer, grit::SoftKneeGainComputer),
    (float, double)
)
{
    using Computer = TestType;
    using Float    = typename Computer::SampleType;

    SECTION("default")
    {
        auto computer = Computer{};
        REQUIRE_THAT(computer(Float(+1)), Catch::Matchers::WithinAbs(+1, 1e-6));
        REQUIRE_THAT(computer(Float(+0)), Catch::Matchers::WithinAbs(+0, 1e-6));
        REQUIRE_THAT(computer(Float(-1)), Catch::Matchers::WithinAbs(-1, 1e-6));
        REQUIRE_THAT(computer(Float(-20)), Catch::Matchers::WithinAbs(-20, 1e-6));
    }

    SECTION("threshold = -6, ratio = 2")
    {
        auto computer = Computer{};
        computer.setParameter({.threshold = grit::Decibels<Float>{-6}, .knee = {}, .ratio = Float(2)});
        REQUIRE_THAT(computer(Float(+2)), Catch::Matchers::WithinAbs(-2, 1e-6));
        REQUIRE_THAT(computer(Float(-2)), Catch::Matchers::WithinAbs(-4, 1e-6));
        REQUIRE_THAT(computer(Float(-4)), Catch::Matchers::WithinAbs(-5, 1e-6));
        REQUIRE_THAT(computer(Float(-6)), Catch::Matchers::WithinAbs(-6, 1e-6));
        REQUIRE_THAT(computer(Float(-6.1)), Catch::Matchers::WithinAbs(-6.1, 1e-6));
        REQUIRE_THAT(computer(Float(-7)), Catch::Matchers::WithinAbs(-7, 1e-6));
        REQUIRE_THAT(computer(Float(-20)), Catch::Matchers::WithinAbs(-20, 1e-6));
    }
}
