#include "ilog2.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE(
    "grit/math: ilog2",
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

    STATIC_REQUIRE(etl::same_as<decltype(grit::ilog2(etl::declval<Int>())), Int>);

    REQUIRE(grit::ilog2(Int(1)) == Int(0));
    REQUIRE(grit::ilog2(Int(2)) == Int(1));
    REQUIRE(grit::ilog2(Int(4)) == Int(2));
    REQUIRE(grit::ilog2(Int(8)) == Int(3));
    REQUIRE(grit::ilog2(Int(16)) == Int(4));
    REQUIRE(grit::ilog2(Int(32)) == Int(5));
    REQUIRE(grit::ilog2(Int(64)) == Int(6));

    if constexpr (sizeof(Int) > 1) {
        REQUIRE(grit::ilog2(Int(128)) == Int(7));
        REQUIRE(grit::ilog2(Int(256)) == Int(8));
        REQUIRE(grit::ilog2(Int(512)) == Int(9));
        REQUIRE(grit::ilog2(Int(1024)) == Int(10));
        REQUIRE(grit::ilog2(Int(2048)) == Int(11));
        REQUIRE(grit::ilog2(Int(4096)) == Int(12));
        REQUIRE(grit::ilog2(Int(8192)) == Int(13));
    }
}
