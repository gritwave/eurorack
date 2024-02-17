#include "normalizable_range.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("math: NormalizableRange", "", float, double)
{
    using Float = TestType;
    using Range = grit::NormalizableRange<Float>;

    STATIC_REQUIRE(etl::same_as<typename Range::value_type, Float>);

    SECTION("empty")
    {
        auto range = Range{};
        REQUIRE(range.getStart() == Float(0));
        REQUIRE(range.getEnd() == Float(0));
        REQUIRE(range.from0to1(Float(0.0)) == Catch::Approx(0.0));
        REQUIRE(range.from0to1(Float(0.5)) == Catch::Approx(0.0));
        REQUIRE(range.from0to1(Float(1.0)) == Catch::Approx(0.0));
    }

    SECTION("0 - 100")
    {
        auto range = Range{Float(0), Float(100)};
        REQUIRE(range.getStart() == Float(0));
        REQUIRE(range.getEnd() == Float(100));

        REQUIRE(range.from0to1(Float(0.0)) == Catch::Approx(0.0));
        REQUIRE(range.from0to1(Float(0.5)) == Catch::Approx(50.0));
        REQUIRE(range.from0to1(Float(1.0)) == Catch::Approx(100.0));

        REQUIRE(range.to0to1(Float(0.0)) == Catch::Approx(0.0));
        REQUIRE(range.to0to1(Float(50.0)) == Catch::Approx(0.5));
        REQUIRE(range.to0to1(Float(100.0)) == Catch::Approx(1.0));
    }

    SECTION("10 - 1000")
    {
        auto range = Range{Float(10), Float(1000)};
        REQUIRE(range.getStart() == Float(10));
        REQUIRE(range.getEnd() == Float(1000));

        REQUIRE(range.from0to1(Float(0.0)) == Catch::Approx(10.0));
        REQUIRE(range.from0to1(Float(0.5)) == Catch::Approx(505.0));
        REQUIRE(range.from0to1(Float(1.0)) == Catch::Approx(1000.0));

        REQUIRE(range.to0to1(Float(10.0)) == Catch::Approx(0.0));
        REQUIRE(range.to0to1(Float(505.0)) == Catch::Approx(0.5));
        REQUIRE(range.to0to1(Float(1000.0)) == Catch::Approx(1.0));
    }

    SECTION("0 - 100 - 25")
    {
        auto range = Range{Float(0), Float(100), Float(25)};
        REQUIRE(range.getStart() == Float(0));
        REQUIRE(range.getEnd() == Float(100));

        REQUIRE(range.from0to1(Float(0.0)) == Catch::Approx(0.0));
        REQUIRE(range.from0to1(Float(0.5)) == Catch::Approx(25.0));
        REQUIRE(range.from0to1(Float(1.0)) == Catch::Approx(100.0));

        REQUIRE(range.to0to1(Float(0.0)) == Catch::Approx(0.0));
        REQUIRE(range.to0to1(Float(25.0)) == Catch::Approx(0.5));
        REQUIRE(range.to0to1(Float(100.0)) == Catch::Approx(1.0));
    }
}
