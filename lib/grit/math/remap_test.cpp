#include "remap.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEMPLATE_TEST_CASE("math: remap", "", float, double)
{
    using Float = TestType;

    REQUIRE_THAT(grit::remap(Float(0.0), Float(0), Float(10)), Catch::Matchers::WithinAbs(0, 1e-6));
    REQUIRE_THAT(grit::remap(Float(0.5), Float(0), Float(10)), Catch::Matchers::WithinAbs(5, 1e-6));
    REQUIRE_THAT(grit::remap(Float(1.0), Float(0), Float(10)), Catch::Matchers::WithinAbs(10, 1e-6));

    REQUIRE_THAT(grit::remap(Float(0.0), Float(-10), Float(10)), Catch::Matchers::WithinAbs(-10, 1e-6));
    REQUIRE_THAT(grit::remap(Float(0.5), Float(-10), Float(10)), Catch::Matchers::WithinAbs(0, 1e-6));
    REQUIRE_THAT(grit::remap(Float(1.0), Float(-10), Float(10)), Catch::Matchers::WithinAbs(10, 1e-6));

    REQUIRE_THAT(grit::remap(Float(0.0), Float(-10), Float(-20)), Catch::Matchers::WithinAbs(-10, 1e-6));
    REQUIRE_THAT(grit::remap(Float(0.5), Float(-10), Float(-20)), Catch::Matchers::WithinAbs(-15, 1e-6));
    REQUIRE_THAT(grit::remap(Float(1.0), Float(-10), Float(-20)), Catch::Matchers::WithinAbs(-20, 1e-6));

    REQUIRE_THAT(
        grit::remap(Float(-10), Float(-10), Float(-20), Float(100), Float(200)),
        Catch::Matchers::WithinAbs(100, 1e-6)
    );
    REQUIRE_THAT(
        grit::remap(Float(-15), Float(-10), Float(-20), Float(100), Float(200)),
        Catch::Matchers::WithinAbs(150, 1e-6)
    );
    REQUIRE_THAT(
        grit::remap(Float(-20), Float(-10), Float(-20), Float(100), Float(200)),
        Catch::Matchers::WithinAbs(200, 1e-6)
    );
}
