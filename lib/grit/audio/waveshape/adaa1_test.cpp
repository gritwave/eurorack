#include "full_wave_rectifier.hpp"
#include "half_wave_rectifier.hpp"
#include "hard_clipper.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_PRODUCT_TEST_CASE(
    "grit/audio/waveshape: Waveshaper",
    "",
    (grit::FullWaveRectifier, grit::HalfWaveRectifier, grit::HardClipper),
    (float, double)
)
{
    using Waveshaper = TestType;
    using Float      = typename Waveshaper::value_type;

    auto shaper = Waveshaper{};
    STATIC_REQUIRE(etl::is_empty_v<Waveshaper>);

    REQUIRE(shaper.processSample(Float(0.1)) == Catch::Approx(0.1));
    REQUIRE(shaper.processSample(Float(0.1)) == Catch::Approx(0.1));

    shaper.reset();
    REQUIRE(shaper.processSample(Float(0.1)) == Catch::Approx(0.1));
    REQUIRE(shaper.processSample(Float(0.1)) == Catch::Approx(0.1));
}

TEMPLATE_PRODUCT_TEST_CASE(
    "grit/audio/waveshape: ADAA1",
    "",
    (grit::FullWaveRectifierADAA1, grit::HalfWaveRectifierADAA1),
    (float, double)
)
{
    using Waveshaper = TestType;
    using Float      = typename Waveshaper::value_type;

    auto shaper = Waveshaper{};
    STATIC_REQUIRE(sizeof(shaper) == sizeof(Float) * 2);

    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.05));
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.1));
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.1));

    shaper.reset();
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.05));
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.1));
    REQUIRE(shaper.processSample(0.1) == Catch::Approx(0.1));
}
