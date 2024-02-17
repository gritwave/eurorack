#include "half_wave_rectifier.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("audio/waveshape: HalfWaveRectifier", "", float, double)
{
    using Float = TestType;

    auto shaper = grit::HalfWaveRectifier<Float>{};
    STATIC_REQUIRE(etl::is_empty_v<grit::HalfWaveRectifier<Float>>);

    REQUIRE(shaper(Float(-2.0)) == Catch::Approx(+0.0));
    REQUIRE(shaper(Float(-1.0)) == Catch::Approx(+0.0));
    REQUIRE(shaper(Float(-0.1)) == Catch::Approx(+0.0));
    REQUIRE(shaper(Float(+0.1)) == Catch::Approx(+0.1));
    REQUIRE(shaper(Float(+1.0)) == Catch::Approx(+1.0));
    REQUIRE(shaper(Float(+2.0)) == Catch::Approx(+2.0));
}
