#include "power.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEMPLATE_TEST_CASE(
    "grit/math: power",
    "",
    short,
    int,
    long,
    long long,
    unsigned short,
    unsigned int,
    unsigned long,
    unsigned long long
)
{
    using Int = TestType;

    REQUIRE(grit::power<1>(Int(1)) == 1);
    REQUIRE(grit::power<1>(Int(2)) == 2);
    REQUIRE(grit::power<1>(Int(3)) == 3);

    REQUIRE(grit::power<2>(Int(1)) == 1);
    REQUIRE(grit::power<2>(Int(2)) == 4);
    REQUIRE(grit::power<2>(Int(3)) == 9);

    REQUIRE(grit::power<3>(Int(1)) == 1);
    REQUIRE(grit::power<3>(Int(2)) == 8);
    REQUIRE(grit::power<3>(Int(3)) == 27);
}

TEMPLATE_TEST_CASE("grit/math: power", "", float, double)
{
    using Float = TestType;

    REQUIRE_THAT(grit::power<1>(Float(1)), Catch::Matchers::WithinAbs(1, 1e-8));
    REQUIRE_THAT(grit::power<1>(Float(2)), Catch::Matchers::WithinAbs(2, 1e-8));
    REQUIRE_THAT(grit::power<1>(Float(3)), Catch::Matchers::WithinAbs(3, 1e-8));

    REQUIRE_THAT(grit::power<2>(Float(1)), Catch::Matchers::WithinAbs(1, 1e-8));
    REQUIRE_THAT(grit::power<2>(Float(2)), Catch::Matchers::WithinAbs(4, 1e-8));
    REQUIRE_THAT(grit::power<2>(Float(3)), Catch::Matchers::WithinAbs(9, 1e-8));

    REQUIRE_THAT(grit::power<3>(Float(1)), Catch::Matchers::WithinAbs(1, 1e-8));
    REQUIRE_THAT(grit::power<3>(Float(2)), Catch::Matchers::WithinAbs(8, 1e-8));
    REQUIRE_THAT(grit::power<3>(Float(3)), Catch::Matchers::WithinAbs(27, 1e-8));
}
