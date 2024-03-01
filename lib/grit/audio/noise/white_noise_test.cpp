#include "white_noise.hpp"

#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("audio/noise: WhiteNoise", "", float, double)
{
    using Float = TestType;

    auto proc = grit::WhiteNoise<Float>{Catch::getSeed()};
    for (auto i{0}; i < 1'000; ++i) {
        REQUIRE(proc() >= Float(-1.0));
        REQUIRE(proc() <= Float(+1.0));
    }
}
