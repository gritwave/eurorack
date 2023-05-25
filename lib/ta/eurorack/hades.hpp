#pragma once

#include <ta/audio/dynamic/compressor.hpp>
#include <ta/audio/envelope/envelope_follower.hpp>
#include <ta/audio/noise/white_noise.hpp>
#include <ta/audio/unit/decibel.hpp>
#include <ta/audio/waveshape/wave_shaper.hpp>
#include <ta/math/dynamic_smoothing.hpp>

#include <etl/array.hpp>
#include <etl/functional.hpp>
#include <etl/span.hpp>

namespace ta
{

struct Hades
{
    struct ControlInputs
    {
        float textureKnob{0};
        float morphKnob{0};
        float ampKnob{0};
        float compressorKnob{0};
        float morphCV{0};
        float sideChainCV{0};
        float attackCV{0};
        float releaseCV{0};

        bool gate1{false};
        bool gate2{false};
    };

    struct ControlOutputs
    {
        float envelope{0};
        bool gate1{false};
        bool gate2{false};
    };

    struct Buffers
    {
        etl::array<etl::span<float const>, 2> input;
        etl::array<etl::span<float>, 2> output;
    };

    Hades() = default;

    auto prepare(float sampleRate, etl::size_t blockSize) -> void;
    [[nodiscard]] auto processBlock(Buffers const& context, ControlInputs const& inputs) -> ControlOutputs;

private:
    struct Channel
    {
        struct Parameter
        {
            float texture{0.0F};
            float morph{0.0F};
            float amp{0.0F};
            float compressor{0.0F};
            float sideChain{0.0F};
            float attack{0.0F};
            float release{0.0F};
        };

        Channel() = default;

        auto setParameter(Parameter const& parameter) -> void;

        auto prepare(float sampleRate) -> void;
        [[nodiscard]] auto processSample(float sample) -> float;

    private:
        Parameter _parameter{};
        EnvelopeFollower<float> _envelopeFollower{};
        WhiteNoise<float> _noise{};
        WaveShaper<float> _waveShaper{std::tanh};
        Compressor<float> _compressor;
    };

    DynamicSmoothing<float> _textureKnob;
    DynamicSmoothing<float> _morphKnob;
    DynamicSmoothing<float> _ampKnob;
    DynamicSmoothing<float> _compressorKnob;
    DynamicSmoothing<float> _morphCV;
    DynamicSmoothing<float> _sideChainCV;
    DynamicSmoothing<float> _attackCV;
    DynamicSmoothing<float> _releaseCV;

    etl::array<Channel, 2> _channels{};
};

}  // namespace ta
