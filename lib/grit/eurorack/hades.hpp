#pragma once

#include <grit/audio/container/stereo_block.hpp>
#include <grit/audio/dynamic/compressor.hpp>
#include <grit/audio/envelope/envelope_follower.hpp>
#include <grit/audio/filter/dynamic_smoothing.hpp>
#include <grit/audio/mix/cross_fade.hpp>
#include <grit/audio/noise/airwindows_vinyl_dither.hpp>
#include <grit/audio/noise/white_noise.hpp>
#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/math/remap.hpp>
#include <grit/unit/decibel.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/functional.hpp>
#include <etl/utility.hpp>

namespace grit {

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
        StereoBlock<float const> input;
        StereoBlock<float> output;
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
        [[nodiscard]] auto operator()(float sample) -> etl::pair<float, float>;

    private:
        Parameter _parameter{};
        EnvelopeFollower<float> _envelopeFollower{};
        WhiteNoise<float> _noise{};
        AirWindowsVinylDither<float> _vinylDither{};
        WaveShaper<float> _waveShaper{etl::tanh};
        Compressor<float> _compressor{};
    };

    DynamicSmoothing<float> _textureKnob{};
    DynamicSmoothing<float> _morphKnob{};
    DynamicSmoothing<float> _ampKnob{};
    DynamicSmoothing<float> _compressorKnob{};
    DynamicSmoothing<float> _morphCv{};
    DynamicSmoothing<float> _sideChainCv{};
    DynamicSmoothing<float> _attackCv{};
    DynamicSmoothing<float> _releaseCv{};

    etl::array<Channel, 2> _channels{};
};

}  // namespace grit
