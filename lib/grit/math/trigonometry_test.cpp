#include "trigonometry.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEMPLATE_TEST_CASE("math: bhaskara", "", float, double)
{
    using Float = TestType;

    REQUIRE_THAT(grit::bhaskara(Float(0.0)), Catch::Matchers::WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(grit::bhaskara(Float(0.5)), Catch::Matchers::WithinAbs(0.479426, 1e-2));
    REQUIRE_THAT(grit::bhaskara(Float(1.0)), Catch::Matchers::WithinAbs(0.841471, 1e-2));
    REQUIRE_THAT(grit::bhaskara(Float(etl::numbers::pi / 3.0)), Catch::Matchers::WithinAbs(0.866025, 1e-2));
    REQUIRE_THAT(grit::bhaskara(Float(etl::numbers::pi / 2.0)), Catch::Matchers::WithinAbs(1.0, 1e-8));
}
