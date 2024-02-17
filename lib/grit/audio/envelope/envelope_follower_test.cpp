#include "envelope_follower.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("grit/audio/envelope: EnvelopeFollower", "", float, double)
{
    using Float = TestType;

    auto follower = grit::EnvelopeFollower<Float>{};
    follower.setSampleRate(Float(44'100));
    REQUIRE(follower(Float(0)) == Catch::Approx(Float(0)));
    REQUIRE(follower(Float(0)) == Catch::Approx(Float(0)));

    auto const x1 = follower(Float(0.25));
    REQUIRE(x1 > Float(0));
    REQUIRE(x1 < Float(0.25));

    auto const x2 = follower(Float(0.25));
    REQUIRE(x2 > x1);
    REQUIRE(x2 < Float(0.25));

    auto const x3 = follower(Float(x1));
    REQUIRE(x3 < x2);
    REQUIRE(x3 < Float(0.25));
    REQUIRE(x3 > Float(x1));

    follower.reset();
    follower.setParameter({grit::Milliseconds<Float>{12}});
    REQUIRE(follower(Float(0)) == Catch::Approx(Float(0)));

    auto const y1 = follower(Float(0.25));
    REQUIRE(y1 > Float(0));
    REQUIRE(y1 < Float(0.25));
    REQUIRE(y1 > x1);

    auto const y2 = follower(Float(0.25));
    REQUIRE(y2 > y1);
    REQUIRE(y2 < Float(0.25));
    REQUIRE(y2 > x2);

    auto const y3 = follower(Float(y1));
    REQUIRE(y3 < y2);
    REQUIRE(y3 < Float(0.25));
    REQUIRE(y3 > Float(y1));
}
