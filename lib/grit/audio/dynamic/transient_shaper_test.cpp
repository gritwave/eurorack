#include "transient_shaper.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("grit/audio/dynamic: TransientShaper", "", float, double)
{
    using Float = TestType;

    auto shaper = grit::TransientShaper<Float>{};
    shaper.prepare(Float(44'100));
    REQUIRE(shaper(Float(0)) == Catch::Approx(Float(0)));
    REQUIRE(shaper(Float(0)) == Catch::Approx(Float(0)));
    REQUIRE(shaper(Float(0.25)) == Catch::Approx(Float(0.25)));

    shaper.setParameter({Float(0.25), Float(0.1)});
    REQUIRE(shaper(Float(0.30)) > Float(0.30));
    REQUIRE(shaper(Float(0.32)) > Float(0.32));

    shaper.reset();
    shaper.setParameter({Float(0), Float(0)});
    REQUIRE(shaper(Float(0)) == Catch::Approx(Float(0)));
    REQUIRE(shaper(Float(0.25)) == Catch::Approx(Float(0.25)));
}
