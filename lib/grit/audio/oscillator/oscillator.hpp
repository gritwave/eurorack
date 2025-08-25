#pragma once

#include <grit/math/remap.hpp>

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>
#include <etl/cstdint.hpp>
#include <etl/numbers.hpp>

namespace grit {

/// \ingroup grit-audio-oscillator
enum struct OscillatorShape : etl::uint8_t
{
    Sine,
    Triangle,
    Square,
};

/// \ingroup grit-audio-oscillator
template<etl::floating_point Float>
struct Oscillator
{
    Oscillator() = default;

    auto setShape(OscillatorShape shape) -> void;
    auto setPhase(Float phase) -> void;
    auto setFrequency(Float frequency) -> void;
    auto setSampleRate(Float sampleRate) -> void;

    auto addPhaseOffset(Float offset) -> void;

    [[nodiscard]] auto operator()() -> Float;

private:
    [[nodiscard]] static auto sine(Float phase) -> Float;
    [[nodiscard]] static auto triangle(Float phase) -> Float;
    [[nodiscard]] static auto pulse(Float phase, Float width) -> Float;

    OscillatorShape _shape{OscillatorShape::Sine};
    Float _sampleRate{0};
    Float _phase{0};
    Float _phaseIncrement{0};
    Float _pulseWidth{0.5};
};

template<etl::floating_point Float>
auto Oscillator<Float>::setShape(OscillatorShape shape) -> void
{
    _shape = shape;
}

template<etl::floating_point Float>
auto Oscillator<Float>::setPhase(Float phase) -> void
{
    _phase = phase;
}

template<etl::floating_point Float>
auto Oscillator<Float>::setFrequency(Float frequency) -> void
{
    _phaseIncrement = 1.0F / (_sampleRate / frequency);
}

template<etl::floating_point Float>
auto Oscillator<Float>::setSampleRate(Float sampleRate) -> void
{
    _sampleRate = sampleRate;
}

template<etl::floating_point Float>
auto Oscillator<Float>::addPhaseOffset(Float offset) -> void
{
    _phase += offset;
    _phase -= etl::floor(_phase);
}

template<etl::floating_point Float>
auto Oscillator<Float>::operator()() -> Float
{
    auto output = Float{};
    switch (_shape) {
        case OscillatorShape::Sine: {
            output = sine(_phase);
            break;
        }
        case OscillatorShape::Triangle: {
            output = triangle(_phase);
            break;
        }
        case OscillatorShape::Square: {
            output = pulse(_phase, _pulseWidth);
            break;
        }
        default: {
            break;
        }
    }

    addPhaseOffset(_phaseIncrement);
    return output;
}

template<etl::floating_point Float>
auto Oscillator<Float>::sine(Float phase) -> Float
{
    static constexpr auto twoPi = static_cast<Float>(etl::numbers::pi) * Float{2};
    return etl::sin(phase * twoPi);
}

template<etl::floating_point Float>
auto Oscillator<Float>::triangle(Float phase) -> Float
{
    auto const x = phase <= Float{0.5} ? phase : Float{1} - phase;
    return (x - Float{0.25}) * Float{4};
}

template<etl::floating_point Float>
auto Oscillator<Float>::pulse(Float phase, Float width) -> Float
{
    auto const w = etl::clamp(width, Float{0}, Float{1});
    if (phase < w) {
        return Float{-1};
    }
    return Float{1};
}

}  // namespace grit
