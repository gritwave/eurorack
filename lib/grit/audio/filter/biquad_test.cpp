#include "biquad.hpp"

#include <etl/random.hpp>

#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEMPLATE_TEST_CASE("audio/filter: BiquadCoefficients::makeBypass", "", float, double)
{
    using Float        = TestType;
    using Coefficients = grit::BiquadCoefficients<Float>;

    static constexpr auto bypass = Coefficients::makeBypass();
    STATIC_REQUIRE(bypass.size() == 6U);

    REQUIRE_THAT(bypass[0], Catch::Matchers::WithinAbs(1.0, 1e-6));
    REQUIRE_THAT(bypass[1], Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(bypass[2], Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(bypass[3], Catch::Matchers::WithinAbs(1.0, 1e-6));
    REQUIRE_THAT(bypass[4], Catch::Matchers::WithinAbs(0.0, 1e-6));
    REQUIRE_THAT(bypass[5], Catch::Matchers::WithinAbs(0.0, 1e-6));
}

TEMPLATE_TEST_CASE("audio/filter: BiquadTDF2", "", float, double)
{
    using Float  = TestType;
    using Filter = grit::BiquadTDF2<Float>;

    static constexpr auto iterations = 1'000;

    auto rng  = etl::xoshiro128plusplus{Catch::getSeed()};
    auto dist = etl::uniform_real_distribution<Float>{Float(-1), Float(+1)};

    SECTION("default coefficents are bypass")
    {
        auto filter = Filter{};
        for (auto i{0}; i < iterations; ++i) {
            auto const x = dist(rng);
            auto const y = filter(x);
            REQUIRE_THAT(y, Catch::Matchers::WithinAbs(x, 1e-6));
        }

        filter.setCoefficients(Filter::Coefficients::makeBypass());
        filter.reset();

        for (auto i{0}; i < iterations; ++i) {
            auto const x = dist(rng);
            auto const y = filter(x);
            REQUIRE_THAT(y, Catch::Matchers::WithinAbs(x, 1e-6));
        }
    }
}
