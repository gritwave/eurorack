#pragma once

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
#include <etl/span.hpp>

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
        etl::array<etl::span<float const>, 2> input{};
        etl::array<etl::span<float>, 2> output{};
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
        [[nodiscard]] auto operator()(float sample) -> float;

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

inline auto Hades::Channel::setParameter(Parameter const& parameter) -> void
{
    _parameter = parameter;

    auto const attack  = Milliseconds<float>{remap(parameter.attack, 1.0F, 50.0F)};
    auto const release = Milliseconds<float>{remap(parameter.release, 1.0F, 500.0F)};

    _envelopeFollower.setParameter({
        .attack  = attack,
        .release = release,
    });

    _compressor.setParameter({
        .threshold = remap(parameter.compressor, -12.0F, -20.0F),
        .ratio     = remap(parameter.compressor, +4.0F, +10.0F),
        .knee      = 2.0F,
        .attack    = attack,
        .release   = release,
        .makeUp    = 1.0F,
        .wet       = 1.0F,
    });
}

inline auto Hades::Channel::operator()(float sample) -> float
{
    auto const env = _envelopeFollower(sample);
    _vinylDither.setDeRez(etl::clamp(env, 0.0F, 1.0F));

    auto const noise = _noise();
    auto const vinyl = _vinylDither(sample);
    auto const mix   = _parameter.morph;
    auto const mixed = (noise * mix) + (vinyl * (1.0F - mix));

    auto const drive   = remap(_parameter.amp, 1.0F, 4.0F);  // +12dB
    auto const distOut = _waveShaper(vinyl * drive);
    return _compressor(distOut, distOut);
}

inline auto Hades::Channel::prepare(float sampleRate) -> void
{
    _envelopeFollower.prepare(sampleRate);
    _compressor.prepare(sampleRate);
}

inline auto Hades::prepare(float sampleRate, etl::size_t blockSize) -> void
{
    auto const blockRate = sampleRate / static_cast<float>(blockSize);

    _textureKnob.prepare(blockRate);
    _morphKnob.prepare(blockRate);
    _ampKnob.prepare(blockRate);
    _compressorKnob.prepare(blockRate);
    _morphCv.prepare(blockRate);
    _sideChainCv.prepare(blockRate);
    _attackCv.prepare(blockRate);
    _releaseCv.prepare(blockRate);

    _channels[0].prepare(sampleRate);
    _channels[1].prepare(sampleRate);
}

inline auto Hades::processBlock(Buffers const& context, ControlInputs const& inputs) -> ControlOutputs
{
    auto const textureKnob    = _textureKnob.process(inputs.textureKnob);
    auto const morphKnob      = _morphKnob.process(inputs.morphKnob);
    auto const ampKnob        = _ampKnob.process(inputs.ampKnob);
    auto const compressorKnob = _compressorKnob.process(inputs.compressorKnob);
    auto const morphCv        = _morphCv.process(inputs.morphCV);
    auto const sideChainCv    = _sideChainCv.process(inputs.sideChainCV);
    auto const attackCv       = _attackCv.process(inputs.attackCV);
    auto const releaseCv      = _releaseCv.process(inputs.releaseCV);

    auto const channelParameter = Hades::Channel::Parameter{
        .texture    = textureKnob,
        .morph      = etl::clamp(morphKnob + morphCv, 0.0F, 1.0F),
        .amp        = ampKnob,
        .compressor = compressorKnob,
        .sideChain  = sideChainCv,
        .attack     = attackCv,
        .release    = releaseCv,
    };

    for (auto& channel : _channels) {
        channel.setParameter(channelParameter);
    }

    for (size_t i = 0; i < context.input[0].size(); ++i) {
        auto const left  = context.input[0][i];
        auto const right = context.input[1][i];

        context.output[0][i] = _channels[0](left);
        context.output[1][i] = _channels[1](right);
    }

    // "DIGITAL" GATE LOGIC
    auto const gateOut = inputs.gate1 != inputs.gate2;

    return {
        .envelope = 0.0F,
        .gate1    = gateOut,
        .gate2    = not gateOut,
    };
}

}  // namespace grit
