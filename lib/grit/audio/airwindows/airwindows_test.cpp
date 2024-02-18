#include <grit/audio/airwindows.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

template<typename Processor>
auto test(auto sampleRate) -> void
{
    using Float = typename Processor::value_type;

    CAPTURE(sampleRate);

    auto rng    = etl::xoshiro128plusplus{Catch::getSeed()};
    auto param  = etl::uniform_real_distribution<Float>{Float(0), Float(1)};
    auto signal = etl::uniform_real_distribution<Float>{Float(-1), Float(+1)};
    auto proc   = Processor{rng()};

    if constexpr (requires { proc.setSampleRate(sampleRate); }) {
        proc.setSampleRate(sampleRate);
    }

    SECTION("check for nans")
    {
        for (auto p{0}; p < 10; ++p) {
            proc.reset();

            if constexpr (requires { proc.setParameter({param(rng), param(rng), param(rng), param(rng)}); }) {
                proc.setParameter({param(rng), param(rng), param(rng), param(rng)});
            } else if constexpr (requires { proc.setDeRez(param(rng)); }) {
                proc.setDeRez(param(rng));
            }

            for (auto i{0}; i < static_cast<int>(sampleRate); ++i) {
                auto out = proc(signal(rng));
                REQUIRE(etl::isfinite(out));
            }
        }
    }
}

TEMPLATE_TEST_CASE("audio/airwindows: AirWindowsFireAmp", "", float, double)
{
    using Float = TestType;

    auto const sampleRate = GENERATE(Float(22050), Float(44100), Float(48000), Float(88200), Float(96000));
    test<grit::AirWindowsFireAmp<TestType>>(sampleRate);
}

TEMPLATE_TEST_CASE("audio/airwindows: AirWindowsGrindAmp", "", float, double)
{
    using Float = TestType;
    auto const sampleRate
        = GENERATE(Float(22050), Float(44100), Float(48000), Float(88200), Float(96000), Float(192000));
    test<grit::AirWindowsGrindAmp<TestType>>(sampleRate);
}

TEMPLATE_TEST_CASE("audio/airwindows: AirWindowsVinylDither", "", float, double)
{
    using Float = TestType;

    auto proc = grit::AirWindowsVinylDither<Float>{42};
    proc.reset();
    REQUIRE(proc(Float(0.0)) == Catch::Approx(Float(0.0)));

    proc.setDeRez(Float(0.5));
    REQUIRE(proc.getDeRez() == Catch::Approx(Float(0.5)));

    test<grit::AirWindowsVinylDither<Float>>(Float(44'100));
}
