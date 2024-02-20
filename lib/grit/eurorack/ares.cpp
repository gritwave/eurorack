#include "ares.hpp"

#include <etl/algorithm.hpp>
#include <etl/functional.hpp>

namespace grit {

auto Ares::prepare(float sampleRate, etl::size_t blockSize) -> void
{
    auto const blockRate = sampleRate / static_cast<float>(blockSize);

    _gainKnob.setSampleRate(blockRate);
    _toneKnob.setSampleRate(blockRate);
    _outputKnob.setSampleRate(blockRate);
    _mixKnob.setSampleRate(blockRate);
    _gainCV.setSampleRate(blockRate);
    _toneCV.setSampleRate(blockRate);
    _outputCV.setSampleRate(blockRate);
    _mixCV.setSampleRate(blockRate);

    _channels[0].setSampleRate(sampleRate);
    _channels[1].setSampleRate(sampleRate);
}

auto Ares::process(StereoBlock<float> const& buffer, ControlInput const& inputs) -> void
{
    auto const gainKnob   = _gainKnob(inputs.gainKnob);
    auto const toneKnob   = _toneKnob(inputs.toneKnob);
    auto const outputKnob = _outputKnob(inputs.outputKnob);
    auto const mixKnob    = _mixKnob(inputs.mixKnob);

    auto const gainCV   = _gainCV(inputs.gainCV);
    auto const toneCV   = _toneCV(inputs.toneCV);
    auto const outputCV = _outputCV(inputs.outputCV);
    auto const mixCV    = _mixCV(inputs.mixCV);

    auto const parameter = Channel::Parameter{
        .mode   = inputs.mode,
        .gain   = etl::clamp(gainKnob + gainCV, 0.0F, 1.0F),
        .tone   = etl::clamp(toneKnob + toneCV, 0.0F, 1.0F),
        .output = etl::clamp(outputKnob + outputCV, 0.0F, 1.0F),
        .mix    = etl::clamp(mixKnob + mixCV, 0.0F, 1.0F),
    };

    for (auto& channel : _channels) {
        channel.setParameter(parameter);
    }

    for (auto i = size_t(0); i < buffer.extent(1); ++i) {
        buffer(0, i) = etl::invoke(_channels[0], buffer(0, i));
        buffer(1, i) = etl::invoke(_channels[1], buffer(1, i));
    }
}

auto Ares::Channel::setParameter(Parameter const& parameter) -> void
{
    if (parameter.mode != _mode) {
        _fire.reset();
        _grind.reset();
    }

    _mode = parameter.mode;

    if (parameter.mode == Mode::Fire) {
        _fire.setParameter({parameter.gain, parameter.tone, parameter.output, parameter.mix});
    } else {
        _grind.setParameter({parameter.gain, parameter.tone, parameter.output, parameter.mix});
    }
}

auto Ares::Channel::setSampleRate(float sampleRate) -> void
{
    _fire.setSampleRate(sampleRate);
    _grind.setSampleRate(sampleRate);
}

auto Ares::Channel::operator()(float sample) -> float
{
    if (_mode == Mode::Fire) {
        return _fire(sample);
    }
    return _grind(sample);
}

}  // namespace grit
