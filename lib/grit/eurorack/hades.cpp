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

    _textureKnob.setSampleRate(blockRate);
    _morphKnob.setSampleRate(blockRate);
    _ampKnob.setSampleRate(blockRate);
    _compressorKnob.setSampleRate(blockRate);
    _morphCv.setSampleRate(blockRate);
    _sideChainCv.setSampleRate(blockRate);
    _attackCv.setSampleRate(blockRate);
    _releaseCv.setSampleRate(blockRate);

    _channels[0].setSampleRate(sampleRate);
    _channels[1].setSampleRate(sampleRate);
}

auto Hades::process(StereoBlock<float> const& buffer, ControlInput const& inputs) -> ControlOutput
{
    auto const textureKnob    = _textureKnob(inputs.textureKnob);
    auto const morphKnob      = _morphKnob(inputs.morphKnob);
    auto const ampKnob        = _ampKnob(inputs.ampKnob);
    auto const compressorKnob = _compressorKnob(inputs.compressorKnob);
    auto const morphCv        = _morphCv(inputs.morphCV);
    auto const sideChainCv    = _sideChainCv(inputs.sideChainCV);
    auto const attackCv       = _attackCv(inputs.attackCV);
    auto const releaseCv      = _releaseCv(inputs.releaseCV);

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
    for (auto i = size_t(0); i < buffer.extent(1); ++i) {
        auto const [left, envLeft]   = etl::invoke(_channels[0], buffer(0, i));
        auto const [right, envRight] = etl::invoke(_channels[1], buffer(1, i));

        env = (envLeft + envRight) * 0.5F;

        buffer(0, i) = left;
        buffer(1, i) = right;
    }

    // "DIGITAL" GATE LOGIC
    auto const gateOut = inputs.gate1 != inputs.gate2;

    return {
        .envelope = env,
        .gate1    = gateOut,
        .gate2    = not gateOut,
    };
}

auto Hades::Amp::next() -> void
{
    _index = Index{int(_index) + 1};
    if (_index == MaxIndex) {
        _index = Index{0};
    }
}

auto Hades::Amp::setSampleRate(float sampleRate) -> void
{
    _fireAmp.setSampleRate(sampleRate);
    _grindAmp.setSampleRate(sampleRate);
}

auto Hades::Amp::operator()(float sample) -> float
{
    switch (_index) {
        case TanhIndex: return _tanh(sample);
        case HardIndex: return _hard(sample);
        case FullWaveIndex: return _fullWave(sample);
        case HalfWaveIndex: return _halfWave(sample);
        case DiodeIndex: return _diode(sample);
        case FireAmpIndex: return _fireAmp(sample);
        case GrindAmpIndex: return _grindAmp(sample);
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
        .threshold = Decibels<float>{remap(parameter.compressor, -6.0F, -12.0F)},
        .knee      = Decibels<float>{2.0F},
        .ratio     = remap(parameter.compressor, +1.0F, +8.0F),
        .attack    = attack,
        .release   = release,
    });
}

auto Hades::Channel::nextDistortionAlgorithm() -> void { _distortion.next(); }

auto Hades::Channel::setSampleRate(float sampleRate) -> void
{
    _envelope.setSampleRate(sampleRate);
    _compressor.setSampleRate(sampleRate);
    _distortion.setSampleRate(sampleRate);
}

auto Hades::Channel::operator()(float sample) -> etl::pair<float, float>
{
    auto const env     = _envelope(sample);
    auto const texture = etl::clamp(env + _parameter.texture, 0.0F, 1.0F);

    // _vinyl.setDeRez(texture);
    // auto const vinyl = _vinyl(sample);

    auto const noise = _whiteNoise() * 0.05F * _parameter.morph * texture;
    // auto const mix   = ;
    // auto const mixed = (noise * mix) + (vinyl * (1.0F - mix));

    auto const drive   = remap(_parameter.amp, 1.0F, 8.0F);  // +18dB
    auto const distOut = _distortion((sample + noise) * drive);
    return {_compressor(distOut, distOut), env};
}

}  // namespace grit
