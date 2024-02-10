#include "full_wave_rectifier.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("grit/audio/waveshape: FullWaveRectifierADAA1", "", float, double)
{
    using Float = TestType;

    auto shaper = grit::FullWaveRectifierADAA1<Float>{};
    STATIC_REQUIRE(sizeof(shaper) == sizeof(Float) * 2);

    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.05));
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.1));
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.1));

    shaper.reset();
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.05));
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.1));
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.1));
}
