#include "ipow.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE(
    "grit/math: ipow",
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

    REQUIRE(grit::ipow<Int>(1, 0) == 1);
    REQUIRE(grit::ipow<Int>(1, 1) == 1);
    REQUIRE(grit::ipow<Int>(1, 2) == 1);

    REQUIRE(grit::ipow<Int>(2, 0) == 1);
    REQUIRE(grit::ipow<Int>(2, 1) == 2);
    REQUIRE(grit::ipow<Int>(2, 2) == 4);

    REQUIRE(grit::ipow<Int(1)>(Int(0)) == 1);
    REQUIRE(grit::ipow<Int(1)>(Int(1)) == 1);
    REQUIRE(grit::ipow<Int(1)>(Int(2)) == 1);

    REQUIRE(grit::ipow<Int(2)>(Int(0)) == 1);
    REQUIRE(grit::ipow<Int(2)>(Int(1)) == 2);
    REQUIRE(grit::ipow<Int(2)>(Int(2)) == 4);
}
