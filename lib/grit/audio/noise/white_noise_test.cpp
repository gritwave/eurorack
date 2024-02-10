#include "white_noise.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("grit/audio/noise: WhiteNoise", "", float, double)
{
    using Float = TestType;

    auto proc = grit::WhiteNoise<Float>{42};

    proc.setGain(Float(0.5));
    REQUIRE(proc.getGain() == Catch::Approx(Float(0.5)));

    for (auto i{0}; i < 1'000; ++i) {
        REQUIRE(proc.processSample() >= Float(-1.0 * 0.5));
        REQUIRE(proc.processSample() <= Float(+1.0 * 0.5));
    }
}
