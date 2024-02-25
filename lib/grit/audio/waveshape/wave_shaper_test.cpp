#include "full_wave_rectifier.hpp"
#include "half_wave_rectifier.hpp"
#include "hard_clipper.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_PRODUCT_TEST_CASE(
    "audio/waveshape: WaveShaper",
    "",
    (grit::FullWaveRectifier, grit::HalfWaveRectifier, grit::HardClipper),
    (float, double)
)
{
    using WaveShaper = TestType;
    using Float      = typename WaveShaper::SampleType;

    auto shaper = WaveShaper{};
    STATIC_REQUIRE(etl::is_empty_v<WaveShaper>);

    REQUIRE(shaper(Float(0.1)) == Catch::Approx(0.1));
    REQUIRE(shaper(Float(0.1)) == Catch::Approx(0.1));

    shaper.reset();
    REQUIRE(shaper(Float(0.1)) == Catch::Approx(0.1));
    REQUIRE(shaper(Float(0.1)) == Catch::Approx(0.1));
}
