#pragma once

#include <gw/audio/mix/cross_fade.hpp>
#include <gw/audio/oscillator/oscillator.hpp>
#include <gw/math/range.hpp>

#include <etl/concepts.hpp>

namespace gw
{

template<etl::floating_point SampleType>
struct VariableShapeOscillator
{
    VariableShapeOscillator() = default;

    auto setShapes(OscillatorShape a, OscillatorShape b) noexcept -> void;
    auto setShapeMorph(SampleType morph) noexcept -> void;

    auto setPhase(SampleType phase) noexcept -> void;
    auto setFrequency(SampleType frequency) noexcept -> void;
    auto setSampleRate(SampleType sampleRate) noexcept -> void;

    auto addPhaseOffset(SampleType offset) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> SampleType;

private:
    Oscillator<SampleType> _oscA{};
    Oscillator<SampleType> _oscB{};
    CrossFade<SampleType> _crossFade;
};

template<etl::floating_point SampleType>
auto VariableShapeOscillator<SampleType>::setShapes(OscillatorShape a, OscillatorShape b) noexcept -> void
{
    _oscA.setShape(a);
    _oscB.setShape(b);
}

template<etl::floating_point SampleType>
auto VariableShapeOscillator<SampleType>::setShapeMorph(SampleType morph) noexcept -> void
{
    _crossFade.setParameter({
        .mix   = etl::clamp(morph, SampleType{0}, SampleType{1}),
        .curve = CrossFadeCurve::ConstantPower,
    });
}

template<etl::floating_point SampleType>
auto VariableShapeOscillator<SampleType>::setPhase(SampleType phase) noexcept -> void
{
    _oscA.setPhase(phase);
    _oscB.setPhase(phase);
}

template<etl::floating_point SampleType>
auto VariableShapeOscillator<SampleType>::setFrequency(SampleType frequency) noexcept -> void
{
    _oscA.setFrequency(frequency);
    _oscB.setFrequency(frequency);
}

template<etl::floating_point SampleType>
auto VariableShapeOscillator<SampleType>::setSampleRate(SampleType sampleRate) noexcept -> void
{
    _oscA.setSampleRate(sampleRate);
    _oscB.setSampleRate(sampleRate);
}

template<etl::floating_point SampleType>
auto VariableShapeOscillator<SampleType>::addPhaseOffset(SampleType offset) noexcept -> void
{
    _oscA.addPhaseOffset(offset);
    _oscB.addPhaseOffset(offset);
}

template<etl::floating_point SampleType>
auto VariableShapeOscillator<SampleType>::operator()() noexcept -> SampleType
{
    return _crossFade.process(_oscA(), _oscB());
}

}  // namespace gw
