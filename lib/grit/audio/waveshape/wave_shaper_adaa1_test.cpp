#include "full_wave_rectifier.hpp"
#include "half_wave_rectifier.hpp"
#include "hard_clipper.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_PRODUCT_TEST_CASE(
    "audio/waveshape: WaveShaperADAA1",
    "",
    (grit::FullWaveRectifierADAA1, grit::HalfWaveRectifierADAA1, grit::HardClipperADAA1),
    (float, double)
)
{
    using Waveshaper = TestType;
    using Float      = typename Waveshaper::SampleType;

    auto shaper = Waveshaper{};
    STATIC_REQUIRE(sizeof(shaper) == sizeof(Float) * 2);

    REQUIRE(shaper(0.1) == Catch::Approx(0.05));
    REQUIRE(shaper(0.1) == Catch::Approx(0.1));
    REQUIRE(shaper(0.1) == Catch::Approx(0.1));

    shaper.reset();
    REQUIRE(shaper(0.1) == Catch::Approx(0.05));
    REQUIRE(shaper(0.1) == Catch::Approx(0.1));
    REQUIRE(shaper(0.1) == Catch::Approx(0.1));
}
