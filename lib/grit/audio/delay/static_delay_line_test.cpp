#include "static_delay_line.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE(
    "grit/audio/delay: StaticDelayLine",
    "",
    grit::BufferInterpolation::None,
    grit::BufferInterpolation::Linear,
    grit::BufferInterpolation::Hermite
)
{
    using Interpolator = TestType;
    using Float        = float;

    auto delay = grit::StaticDelayLine<Float, 64, Interpolator>{};

    delay.setDelay(Float(1.0));
    REQUIRE(delay.popSample() == Catch::Approx(Float(0.0)));
    delay.pushSample(Float(1.0));
    REQUIRE(delay.popSample() == Catch::Approx(Float(1.0)));

    delay.pushSample(Float(1.0));
    delay.reset();
    REQUIRE(delay.popSample() == Catch::Approx(Float(0.0)));
}
