#include "stereo_frame.hpp"

#include <etl/concepts.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>

using namespace grit;

TEMPLATE_TEST_CASE("audio/stereo: StereoFrame", "[stereo]", float, double)
{
    using T = TestType;
    STATIC_REQUIRE(etl::same_as<typename StereoFrame<T>::value_type, T>);
}

TEMPLATE_TEST_CASE("audio/stereo: operator+(StereoFrame)", "[stereo]", float, double)
{
    using T           = TestType;
    auto const result = StereoFrame<T>{T(1), T(2)} + T(1);
    REQUIRE(result.left == Catch::Approx(2));
    REQUIRE(result.right == Catch::Approx(3));
}

TEMPLATE_TEST_CASE("audio/stereo: operator-(StereoFrame)", "[stereo]", float, double)
{
    using T           = TestType;
    auto const result = StereoFrame<T>{T(1), T(2)} - T(1);
    REQUIRE(result.left == Catch::Approx(0));
    REQUIRE(result.right == Catch::Approx(1));
}

TEMPLATE_TEST_CASE("audio/stereo: operator*(StereoFrame)", "[stereo]", float, double)
{
    using T           = TestType;
    auto const result = StereoFrame<T>{T(1), T(2)} * T(2);
    REQUIRE(result.left == Catch::Approx(2));
    REQUIRE(result.right == Catch::Approx(4));
}

TEMPLATE_TEST_CASE("audio/stereo: operator/(StereoFrame)", "[stereo]", float, double)
{
    using T           = TestType;
    auto const result = StereoFrame<T>{T(1), T(2)} / T(2);
    REQUIRE(result.left == Catch::Approx(0.5));
    REQUIRE(result.right == Catch::Approx(1));
}

TEMPLATE_TEST_CASE("audio/stereo: operator+=(StereoFrame)", "[stereo]", float, double)
{
    using T     = TestType;
    auto result = StereoFrame<T>{T(1), T(2)};
    result += T(1);
    REQUIRE(result.left == Catch::Approx(2));
    REQUIRE(result.right == Catch::Approx(3));
}

TEMPLATE_TEST_CASE("audio/stereo: operator-=(StereoFrame)", "[stereo]", float, double)
{
    using T     = TestType;
    auto result = StereoFrame<T>{T(1), T(2)};
    result -= T(1);
    REQUIRE(result.left == Catch::Approx(0));
    REQUIRE(result.right == Catch::Approx(1));
}

TEMPLATE_TEST_CASE("audio/stereo: operator*=(StereoFrame)", "[stereo]", float, double)
{
    using T     = TestType;
    auto result = StereoFrame<T>{T(1), T(2)};
    result *= T(2);
    REQUIRE(result.left == Catch::Approx(2));
    REQUIRE(result.right == Catch::Approx(4));
}

TEMPLATE_TEST_CASE("audio/stereo: operator/=(StereoFrame)", "[stereo]", float, double)
{
    using T     = TestType;
    auto result = StereoFrame<T>{T(1), T(2)};
    result /= T(2);
    REQUIRE(result.left == Catch::Approx(0.5));
    REQUIRE(result.right == Catch::Approx(1));
}
