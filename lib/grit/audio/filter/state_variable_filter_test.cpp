#include "state_variable_filter.hpp"

#include <etl/random.hpp>

#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEMPLATE_PRODUCT_TEST_CASE(
    "audio/filter: StateVariableFilter",
    "",
    (grit::StateVariableHighpass,
     grit::StateVariableBandpass,
     grit::StateVariableLowpass,
     grit::StateVariableNotch,
     grit::StateVariablePeak,
     grit::StateVariableAllpass),
    (float, double)
)
{
    using Filter = TestType;
    using Float  = typename Filter::SampleType;
    STATIC_REQUIRE(etl::same_as<Float, float> or etl::same_as<Float, double>);

    auto rng  = etl::xoshiro128plusplus{Catch::getSeed()};
    auto dist = etl::uniform_real_distribution<Float>{Float(-1), Float(1)};

    auto const fs = GENERATE(Float(1), Float(24000), Float(48000), Float(96000));
    auto filter   = Filter{};
    filter.setSampleRate(fs);
    filter.setParameter({
        .cutoff    = Float(fs * 0.1),
        .resonance = Float(1) / etl::sqrt(Float(2)),
    });

    for (auto i{0}; i < 10'000; ++i) {
        auto const x = dist(rng);
        auto const y = filter(x);

        CAPTURE(i);
        CAPTURE(x);
        CAPTURE(y);
        REQUIRE(etl::isfinite(y));
    }
}
