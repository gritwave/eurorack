#pragma once

#include "../math/range.hpp"
#include "../oscillator/oscillator.hpp"

namespace mc
{

template<typename SampleType>
struct VariableShapeOscillator
{
    VariableShapeOscillator() = default;

    auto setShapes(OscillatorShape a, OscillatorShape b) noexcept -> void;
    auto setShapeMorph(SampleType morph) noexcept -> void;

    auto setPhase(SampleType phase) noexcept -> void;
    auto setFrequency(SampleType frequency) noexcept -> void;
    auto setSampleRate(SampleType sampleRate) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> SampleType;

private:
    Oscillator<SampleType> _oscA{};
    Oscillator<SampleType> _oscB{};
    SampleType _morph{0};
};

template<typename SampleType>
auto VariableShapeOscillator<SampleType>::setShapes(OscillatorShape a, OscillatorShape b) noexcept -> void
{
    _oscA.setShape(a);
    _oscB.setShape(b);
}

template<typename SampleType>
auto VariableShapeOscillator<SampleType>::setShapeMorph(SampleType morph) noexcept -> void
{
    _morph = morph;
}

template<typename SampleType>
auto VariableShapeOscillator<SampleType>::setPhase(SampleType phase) noexcept -> void
{
    _oscA.setPhase(phase);
    _oscB.setPhase(phase);
}

template<typename SampleType>
auto VariableShapeOscillator<SampleType>::setFrequency(SampleType frequency) noexcept -> void
{
    _oscA.setFrequency(frequency);
    _oscB.setFrequency(frequency);
}

template<typename SampleType>
auto VariableShapeOscillator<SampleType>::setSampleRate(SampleType sampleRate) noexcept -> void
{
    _oscA.setSampleRate(sampleRate);
    _oscB.setSampleRate(sampleRate);
}

template<typename SampleType>
auto VariableShapeOscillator<SampleType>::operator()() noexcept -> SampleType
{
    auto const a     = _oscA();
    auto const b     = _oscB();
    auto const morph = clamp(_morph, SampleType{0}, SampleType{1});
    return a * morph + b * (SampleType{1} - morph);
}

}  // namespace mc
