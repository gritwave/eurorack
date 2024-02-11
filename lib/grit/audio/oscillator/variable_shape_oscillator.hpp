#pragma once

#include <grit/audio/mix/cross_fade.hpp>
#include <grit/audio/oscillator/oscillator.hpp>
#include <grit/math/remap.hpp>

#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-audio-oscillator
template<etl::floating_point Float>
struct VariableShapeOscillator
{
    VariableShapeOscillator() = default;

    auto setShapes(OscillatorShape a, OscillatorShape b) -> void;
    auto setShapeMorph(Float morph) -> void;

    auto setPhase(Float phase) -> void;
    auto setFrequency(Float frequency) -> void;
    auto setSampleRate(Float sampleRate) -> void;

    auto addPhaseOffset(Float offset) -> void;

    [[nodiscard]] auto operator()() -> Float;

private:
    Oscillator<Float> _oscA{};
    Oscillator<Float> _oscB{};
    CrossFade<Float> _crossFade;
};

template<etl::floating_point Float>
auto VariableShapeOscillator<Float>::setShapes(OscillatorShape a, OscillatorShape b) -> void
{
    _oscA.setShape(a);
    _oscB.setShape(b);
}

template<etl::floating_point Float>
auto VariableShapeOscillator<Float>::setShapeMorph(Float morph) -> void
{
    _crossFade.set_parameter({
        .mix   = etl::clamp(morph, Float{0}, Float{1}),
        .curve = CrossFadeCurve::ConstantPower,
    });
}

template<etl::floating_point Float>
auto VariableShapeOscillator<Float>::setPhase(Float phase) -> void
{
    _oscA.setPhase(phase);
    _oscB.setPhase(phase);
}

template<etl::floating_point Float>
auto VariableShapeOscillator<Float>::setFrequency(Float frequency) -> void
{
    _oscA.setFrequency(frequency);
    _oscB.setFrequency(frequency);
}

template<etl::floating_point Float>
auto VariableShapeOscillator<Float>::setSampleRate(Float sampleRate) -> void
{
    _oscA.setSampleRate(sampleRate);
    _oscB.setSampleRate(sampleRate);
}

template<etl::floating_point Float>
auto VariableShapeOscillator<Float>::addPhaseOffset(Float offset) -> void
{
    _oscA.addPhaseOffset(offset);
    _oscB.addPhaseOffset(offset);
}

template<etl::floating_point Float>
auto VariableShapeOscillator<Float>::operator()() -> Float
{
    return _crossFade.process(_oscA(), _oscB());
}

}  // namespace grit
