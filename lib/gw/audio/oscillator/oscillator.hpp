#pragma once

#include <gw/math/range.hpp>

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>

namespace gw
{

enum struct OscillatorShape
{
    Sine,
    Triangle,
    Square,
};

template<etl::floating_point SampleType>
struct Oscillator
{
    Oscillator() = default;

    auto setShape(OscillatorShape shape) noexcept -> void;
    auto setPhase(SampleType phase) noexcept -> void;
    auto setFrequency(SampleType frequency) noexcept -> void;
    auto setSampleRate(SampleType sampleRate) noexcept -> void;

    auto addPhaseOffset(SampleType offset) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> SampleType;

private:
    [[nodiscard]] static auto sine(SampleType phase) noexcept -> SampleType;
    [[nodiscard]] static auto triangle(SampleType phase) noexcept -> SampleType;
    [[nodiscard]] static auto pulse(SampleType phase, SampleType width) noexcept -> SampleType;

    OscillatorShape _shape{OscillatorShape::Sine};
    SampleType _sampleRate{0};
    SampleType _phase{0};
    SampleType _phaseIncrement{0};
    SampleType _pulseWidth{0.5};
};

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::setShape(OscillatorShape shape) noexcept -> void
{
    _shape = shape;
}

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::setPhase(SampleType phase) noexcept -> void
{
    _phase = phase;
}

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::setFrequency(SampleType frequency) noexcept -> void
{
    _phaseIncrement = 1.0F / (_sampleRate / frequency);
}

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::setSampleRate(SampleType sampleRate) noexcept -> void
{
    _sampleRate = sampleRate;
}

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::addPhaseOffset(SampleType offset) noexcept -> void
{
    _phase += offset;
    _phase -= etl::floor(_phase);
}

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::operator()() noexcept -> SampleType
{
    auto output = SampleType{};
    switch (_shape)
    {
        case OscillatorShape::Sine:
        {
            output = sine(_phase);
            break;
        }
        case OscillatorShape::Triangle:
        {
            output = triangle(_phase);
            break;
        }
        case OscillatorShape::Square:
        {
            output = pulse(_phase, _pulseWidth);
            break;
        }
        default:
        {
            break;
        }
    }

    addPhaseOffset(_phaseIncrement);
    return output;
}

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::sine(SampleType phase) noexcept -> SampleType
{
    static constexpr auto twoPi = static_cast<SampleType>(etl::numbers::pi) * SampleType{2};
    return etl::sin(phase * twoPi);
}

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::triangle(SampleType phase) noexcept -> SampleType
{
    auto const x = phase <= SampleType{0.5} ? phase : SampleType{1} - phase;
    return (x - SampleType{0.25}) * SampleType{4};
}

template<etl::floating_point SampleType>
auto Oscillator<SampleType>::pulse(SampleType phase, SampleType width) noexcept -> SampleType
{
    auto const w = etl::clamp(width, SampleType{0}, SampleType{1});
    if (phase < w) { return SampleType{-1}; }
    return SampleType{1};
}

}  // namespace gw
