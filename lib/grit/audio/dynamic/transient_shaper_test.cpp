#include "transient_shaper.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("grit/audio/envelope: TransientShaper", "", float, double)
{
    using Float = TestType;

    auto follower = grit::TransientShaper<Float>{};
    follower.prepare(Float(44'100));
    REQUIRE(follower.processSample(Float(0)) == Catch::Approx(Float(0)));
    REQUIRE(follower.processSample(Float(0)) == Catch::Approx(Float(0)));
    REQUIRE(follower.processSample(Float(0.25)) == Catch::Approx(Float(0.25)));

    follower.setParameter({Float(0.25), Float(0.0)});
    REQUIRE(follower.processSample(Float(0.30)) > Float(0.30));
    REQUIRE(follower.processSample(Float(0.32)) > Float(0.32));
}
