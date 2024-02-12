#include "hades.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <etl/algorithm.hpp>
#include <etl/random.hpp>

TEST_CASE("grit/audio/eurorack: Hades")
{
    static constexpr auto sampleRate = 96'000.0F;
    static constexpr auto blockSize  = 32;

    auto input = [] {
        auto buf  = etl::array<float, 2 * blockSize>{};
        auto rng  = etl::xoshiro128plusplus{Catch::getSeed()};
        auto dist = etl::uniform_real_distribution<float>{-1.0F, 1.0F};
        etl::generate(buf.begin(), buf.end(), [&] { return dist(rng); });
        return buf;
    }();

    auto output       = etl::array<float, 2 * blockSize>{};
    auto const buffer = grit::Hades::Buffer{
        .input  = grit::StereoBlock<float const>{ input.data(), blockSize},
        .output = grit::StereoBlock<float>{output.data(), blockSize},
    };

    REQUIRE(buffer.input.extent(0) == 2);
    REQUIRE(buffer.input.extent(1) == blockSize);
    REQUIRE(buffer.output.extent(0) == 2);
    REQUIRE(buffer.output.extent(1) == blockSize);

    auto hades = grit::Hades{};
    hades.prepare(sampleRate, blockSize);

    {
        auto const controls = grit::Hades::ControlInput{
            .textureKnob    = 0.0F,
            .morphKnob      = 0.0F,
            .ampKnob        = 0.0F,
            .compressorKnob = 0.0F,
            .morphCV        = 0.0F,
            .sideChainCV    = 0.0F,
            .attackCV       = 0.0F,
            .releaseCV      = 0.0F,
            .gate1          = false,
            .gate2          = false,
        };

        auto const cv = hades.process(buffer, controls);
        REQUIRE(cv.gate1 == false);
        REQUIRE(cv.gate2 == true);
    }

    {
        auto const controls = grit::Hades::ControlInput{
            .textureKnob    = 0.0F,
            .morphKnob      = 0.0F,
            .ampKnob        = 0.0F,
            .compressorKnob = 0.0F,
            .morphCV        = 0.0F,
            .sideChainCV    = 0.0F,
            .attackCV       = 0.0F,
            .releaseCV      = 0.0F,
            .gate1          = true,
            .gate2          = false,
        };

        hades.nextDistortionAlgorithm();
        auto const cv = hades.process(buffer, controls);
        REQUIRE(cv.gate1 == true);
        REQUIRE(cv.gate2 == false);
    }
}
