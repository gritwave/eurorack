#include "hades.hpp"

namespace grit {

auto Hades::nextTextureAlgorithm() -> void {}

auto Hades::nextDistortionAlgorithm() -> void
{
    for (auto& channel : _channels) {
        channel.nextDistortionAlgorithm();
    }
}

auto Hades::prepare(float sampleRate, etl::size_t blockSize) -> void
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

auto Hades::process(Buffer const& buffer, ControlInput const& inputs) -> ControlOutput
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

    auto env = 0.0F;
    for (auto i = size_t(0); i < buffer.input.extent(1); ++i) {
        auto const [left, envLeft]   = etl::invoke(_channels[0], buffer.input(0, i));
        auto const [right, envRight] = etl::invoke(_channels[1], buffer.input(1, i));

        env = (envLeft + envRight) * 0.5F;

        buffer.output(0, i) = left;
        buffer.output(1, i) = right;
    }

    // "DIGITAL" GATE LOGIC
    auto const gateOut = inputs.gate1 != inputs.gate2;

    return {
        .envelope = env,
        .gate1    = gateOut,
        .gate2    = not gateOut,
    };
}

auto Hades::Distortion::next() -> void
{
    _index = Index{int(_index) + 1};
    if (_index == MaxIndex) {
        _index = Index{0};
    }
}

auto Hades::Distortion::operator()(float sample) -> float
{
    switch (_index) {
        case TanhIndex: return _tanh(sample);
        case HardIndex: return _hard(sample);
        case FullWaveIndex: return _fullWave(sample);
        case HalfWaveIndex: return _halfWave(sample);
        case DiodeIndex: return _diode(sample);
        default: break;
    }
    return sample;
}

auto Hades::Channel::setParameter(Parameter const& parameter) -> void
{
    _parameter = parameter;

    auto const attack  = Milliseconds<float>{attackRange.from0to1(parameter.attack)};
    auto const release = Milliseconds<float>{releaseRange.from0to1(parameter.release)};

    _envelope.setParameter({
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

auto Hades::Channel::nextDistortionAlgorithm() -> void { _distortion.next(); }

auto Hades::Channel::prepare(float sampleRate) -> void
{
    _envelope.prepare(sampleRate);
    _compressor.prepare(sampleRate);
}

auto Hades::Channel::operator()(float sample) -> etl::pair<float, float>
{
    auto const env = _envelope(sample);
    _vinyl.setDeRez(etl::clamp(env, 0.0F, 1.0F));

    auto const noise = _whiteNoise() * 0.5F;
    auto const vinyl = _vinyl(sample);
    auto const mix   = _parameter.morph;
    auto const mixed = (noise * mix) + (vinyl * (1.0F - mix));

    auto const drive   = remap(_parameter.amp, 1.0F, 4.0F);  // +12dB
    auto const distOut = _distortion(mixed * drive);
    return {_compressor(distOut, distOut), env};
}

}  // namespace grit
