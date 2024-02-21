#include "eurorack.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cstddef>
#include <etl/algorithm.hpp>
#include <etl/random.hpp>

TEST_CASE("eurorack: Ares")
{
    static constexpr auto blockSize = 32;

    auto const sampleRate = GENERATE(44100.0F, 48000.0F, 88200.0F, 96000.0F);

    auto rng  = etl::xoshiro128plusplus{Catch::getSeed()};
    auto dist = etl::uniform_real_distribution<float>{-1.0F, 1.0F};

    auto ares = grit::Ares{};
    ares.prepare(sampleRate, blockSize);

    SECTION("fire")
    {
        for (auto i{0}; i < 100; ++i) {
            auto buffer = [&] {
                auto buf = etl::array<float, static_cast<size_t>(2 * blockSize)>{};
                etl::generate(buf.begin(), buf.end(), [&] { return dist(rng); });
                return buf;
            }();
            auto block = grit::StereoBlock<float>{buffer.data(), blockSize};

            ares.process(block, {});

            for (auto i{0}; i < blockSize; ++i) {
                REQUIRE(etl::isfinite(block(0, i)));
                REQUIRE(etl::isfinite(block(1, i)));
            }
        }
    }

    SECTION("grind")
    {

        for (auto i{0}; i < 100; ++i) {
            auto buffer = [&] {
                auto buf = etl::array<float, static_cast<size_t>(2 * blockSize)>{};
                etl::generate(buf.begin(), buf.end(), [&] { return dist(rng); });
                return buf;
            }();
            auto block = grit::StereoBlock<float>{buffer.data(), blockSize};

            auto controls = grit::Ares::ControlInput{};
            controls.mode = grit::Ares::Mode::Grind;
            ares.process(block, controls);

            for (auto i{0}; i < blockSize; ++i) {
                REQUIRE(etl::isfinite(block(0, i)));
                REQUIRE(etl::isfinite(block(1, i)));
            }
        }
    }
}

TEST_CASE("eurorack: Kyma")
{
    static constexpr auto blockSize = 32;

    auto const sampleRate = GENERATE(44100.0F, 48000.0F, 88200.0F, 96000.0F);

    auto rng  = etl::xoshiro128plusplus{Catch::getSeed()};
    auto dist = etl::uniform_real_distribution<float>{-1.0F, 1.0F};

    auto ares = grit::Kyma{};
    ares.prepare(sampleRate, blockSize);

    SECTION("fire")
    {
        for (auto i{0}; i < 100; ++i) {
            auto buffer = [&] {
                auto buf = etl::array<float, static_cast<size_t>(2 * blockSize)>{};
                etl::generate(buf.begin(), buf.end(), [&] { return dist(rng); });
                return buf;
            }();
            auto block = grit::StereoBlock<float>{buffer.data(), blockSize};

            ares.process(block, {});

            for (auto i{0}; i < blockSize; ++i) {
                REQUIRE(etl::isfinite(block(0, i)));
                REQUIRE(etl::isfinite(block(1, i)));
            }
        }
    }
}

TEST_CASE("eurorack: Poseidon")
{
    static constexpr auto blockSize = 32;

    auto const sampleRate = GENERATE(44100.0F, 48000.0F, 88200.0F, 96000.0F);

    auto buffer = [] {
        auto buf  = etl::array<float, static_cast<size_t>(2 * blockSize)>{};
        auto rng  = etl::xoshiro128plusplus{Catch::getSeed()};
        auto dist = etl::uniform_real_distribution<float>{-1.0F, 1.0F};
        etl::generate(buf.begin(), buf.end(), [&] { return dist(rng); });
        return buf;
    }();
    auto block = grit::StereoBlock<float>{buffer.data(), blockSize};

    auto poseidon = grit::Poseidon{};
    poseidon.prepare(sampleRate, blockSize);

    {
        auto const controls = grit::Poseidon::ControlInput{
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

        auto const cv = poseidon.process(block, controls);
        REQUIRE(cv.gate1 == false);
        REQUIRE(cv.gate2 == true);

        for (auto i{0}; i < blockSize; ++i) {
            REQUIRE(etl::isfinite(block(0, i)));
            REQUIRE(etl::isfinite(block(1, i)));
        }
    }

    {
        auto const controls = grit::Poseidon::ControlInput{
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

        poseidon.nextDistortionAlgorithm();
        auto const cv = poseidon.process(block, controls);
        REQUIRE(cv.gate1 == true);
        REQUIRE(cv.gate2 == false);
        for (auto i{0}; i < blockSize; ++i) {
            REQUIRE(etl::isfinite(block(0, i)));
            REQUIRE(etl::isfinite(block(1, i)));
        }
    }

    for (auto i{0}; i < 128; ++i) {
        poseidon.nextDistortionAlgorithm();
        [[maybe_unused]] auto const cv = poseidon.process(block, {});
        for (auto i{0}; i < blockSize; ++i) {
            REQUIRE(etl::isfinite(block(0, i)));
            REQUIRE(etl::isfinite(block(1, i)));
        }
    }
}
