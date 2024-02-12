#include "static_delay_line.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

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
    REQUIRE_THAT(delay.popSample(), Catch::Matchers::WithinAbs(0, 1e-8));
    delay.pushSample(Float(1.0));
    REQUIRE_THAT(delay.popSample(), Catch::Matchers::WithinAbs(1.0, 1e-8));

    delay.pushSample(Float(1.0));
    delay.reset();
    REQUIRE_THAT(delay.popSample(), Catch::Matchers::WithinAbs(0.0, 1e-8));
}
