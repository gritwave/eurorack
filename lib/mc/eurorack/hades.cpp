#include "hades.hpp"

namespace mc
{

auto Hades::Channel::setParameter(Parameter const& parameter) -> void { _parameter = parameter; }

auto Hades::Channel::prepare(float sampleRate) -> void
{
    _envelopeFollower.prepare(sampleRate);
    _compressor.prepare(sampleRate);
}

auto Hades::Channel::processSample(float sample) -> float
{
    _envelopeFollower.setParameter({
        .attack  = Milliseconds<float>{50},
        .release = Milliseconds<float>{50},
    });

    _compressor.setParameter({
        .threshold = decibelsToGain(-12.0F),
        .ratio     = 10.F,
        .knee      = 1.0F,
        .attack    = Milliseconds<float>{50},
        .release   = Milliseconds<float>{50},
        .makeUp    = 1.0F,
        .wet       = 1.0F,
    });

    auto const env     = _envelopeFollower.processSample(sample);
    auto const noise   = _noise.processSample();
    auto const noisy   = sample + noise * env;
    auto const distOut = _waveShaper.processSample(noisy);
    return _compressor.processSample(distOut, distOut);
}

auto Hades::prepare(float sampleRate, etl::size_t blockSize) -> void
{
    auto const blockRate = sampleRate / static_cast<float>(blockSize);

    _textureKnob.prepare(blockRate);
    _morphKnob.prepare(blockRate);
    _ampKnob.prepare(blockRate);
    _compressorKnob.prepare(blockRate);
    _morphCV.prepare(blockRate);
    _sideChainCV.prepare(blockRate);
    _attackCV.prepare(blockRate);
    _releaseCV.prepare(blockRate);

    _channels[0].prepare(sampleRate);
    _channels[1].prepare(sampleRate);
}

auto Hades::processBlock(Buffers const& context, ControlInputs const& inputs) -> ControlOutputs
{
    auto const textureKnob    = _textureKnob.process(inputs.textureKnob);
    auto const morphKnob      = _morphKnob.process(inputs.morphKnob);
    auto const ampKnob        = _ampKnob.process(inputs.ampKnob);
    auto const compressorKnob = _compressorKnob.process(inputs.compressorKnob);
    auto const morphCV        = _morphCV.process(inputs.morphCV);
    auto const sideChainCV    = _sideChainCV.process(inputs.sideChainCV);
    auto const attackCV       = _attackCV.process(inputs.attackCV);
    auto const releaseCV      = _releaseCV.process(inputs.releaseCV);

    auto const channelParameter = Hades::Channel::Parameter{
        .texture    = textureKnob,
        .morph      = etl::clamp(morphKnob + morphCV, 0.0F, 1.0F),
        .amp        = ampKnob,
        .compressor = compressorKnob,
        .sideChain  = sideChainCV,
        .attack     = attackCV,
        .release    = releaseCV,
    };

    for (auto& channel : _channels) { channel.setParameter(channelParameter); }

    for (size_t i = 0; i < context.input[0].size(); ++i)
    {
        auto const left  = context.input[0][i];
        auto const right = context.input[1][i];

        context.output[0][i] = _channels[0].processSample(left);
        context.output[1][i] = _channels[1].processSample(right);
    }

    //  "DIGITAL" GATE LOGIC
    auto const gateOut = inputs.gate1 != inputs.gate2;

    return {
        .envelope = 0.0F,
        .gate1    = gateOut,
        .gate2    = not gateOut,
    };
}

}  // namespace mc
