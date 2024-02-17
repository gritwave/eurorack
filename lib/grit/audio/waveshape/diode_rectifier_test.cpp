#include "diode_rectifier.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("audio/waveshape: DiodeRectifier", "", double)
{
    using Float = TestType;

    auto shaper = grit::DiodeRectifier<Float>{};
    STATIC_REQUIRE(etl::is_empty_v<grit::DiodeRectifier<Float>>);

    REQUIRE(shaper(Float(-2.0)) == Catch::Approx(-0.1944248603));
    REQUIRE(shaper(Float(-1.5)) == Catch::Approx(-0.1863557612));
    REQUIRE(shaper(Float(+0.0)) == Catch::Approx(+0.0));
    REQUIRE(shaper(Float(+1.0)) == Catch::Approx(+0.9978904933));
}

TEMPLATE_TEST_CASE("audio/waveshape: DiodeRectifierADAA1", "", float, double)
{
    using Float = TestType;

    auto shaper = grit::DiodeRectifierADAA1<Float>{};
    REQUIRE(shaper(Float(-2.0)) < Float(0));
    REQUIRE(shaper(Float(-2.0)) == Catch::Approx(-0.1944248603));

    REQUIRE(shaper(Float(-1.5)) < Float(0));
    REQUIRE(shaper(Float(-1.5)) == Catch::Approx(-0.1863557612));
}
