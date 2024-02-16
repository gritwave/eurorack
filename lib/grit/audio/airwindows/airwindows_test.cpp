#include <grit/audio/airwindows.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

template<typename Processor>
auto test() -> void
{
    using Float = typename Processor::value_type;

    auto const sampleRate = GENERATE(Float(44100), Float(48000), Float(88200), Float(96000));

    auto rng  = etl::xoshiro128plusplus{Catch::getSeed()};
    auto dist = etl::uniform_real_distribution<Float>{Float(0), Float(1)};
    auto proc = Processor{rng()};

    if constexpr (requires { proc.setSampleRate(sampleRate); }) {
        proc.setSampleRate(sampleRate);
    }

    SECTION("check for nans")
    {
        for (auto p{0}; p < 10; ++p) {
            proc.reset();

            if constexpr (requires { proc.setParameter({dist(rng), dist(rng), dist(rng), dist(rng)}); }) {
                proc.setParameter({dist(rng), dist(rng), dist(rng), dist(rng)});
            } else if constexpr (requires { proc.setDeRez(dist(rng)); }) {
                proc.setDeRez(dist(rng));
            }

            for (auto i{0}; i < static_cast<int>(sampleRate); ++i) {
                auto out = proc(dist(rng));
                REQUIRE(etl::isfinite(out));
            }
        }
    }
}

TEMPLATE_TEST_CASE("grit/audio/airwindows: AirWindowsFireAmp", "", float, double)
{
    test<grit::AirWindowsFireAmp<TestType>>();
}

TEMPLATE_TEST_CASE("grit/audio/airwindows: AirWindowsGrindAmp", "", float, double)
{
    test<grit::AirWindowsGrindAmp<TestType>>();
}

TEMPLATE_TEST_CASE("grit/audio/airwindows: AirWindowsVinylDither", "", float, double)
{
    using Float = TestType;

    auto proc = grit::AirWindowsVinylDither<Float>{42};
    proc.reset();
    REQUIRE(proc(Float(0.0)) == Catch::Approx(Float(0.0)));

    proc.setDeRez(Float(0.5));
    REQUIRE(proc.getDeRez() == Catch::Approx(Float(0.5)));

    test<grit::AirWindowsVinylDither<Float>>();
}