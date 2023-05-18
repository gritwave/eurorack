#pragma once

#include "constants.hpp"

namespace mc
{

enum struct OscillatorShape
{
    Sine,
    Triangle,
    Saw,
    Square,
};

template<typename SampleType>
struct Oscillator
{
    Oscillator() = default;

    auto setShape(OscillatorShape shape) noexcept -> void;
    auto setPhase(SampleType phase) noexcept -> void;
    auto setFrequency(SampleType frequency) noexcept -> void;
    auto setSampleRate(SampleType sampleRate) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> SampleType;

private:
    [[nodiscard]] auto sine() noexcept -> SampleType;
    [[nodiscard]] auto triangle() noexcept -> SampleType;
    [[nodiscard]] auto saw() noexcept -> SampleType;
    [[nodiscard]] auto pulse(SampleType width) noexcept -> SampleType;

    OscillatorShape _shape{};
    SampleType _phase{0};
    SampleType _frequency{0};
    SampleType _pulseWidth{0.5};
    SampleType _sampleRate{0};
};

template<typename SampleType>
auto Oscillator<SampleType>::setShape(OscillatorShape shape) noexcept -> void
{
    _shape = shape;
}

template<typename SampleType>
auto Oscillator<SampleType>::setPhase(SampleType phase) noexcept -> void
{
    _phase = phase;
}

template<typename SampleType>
auto Oscillator<SampleType>::setFrequency(SampleType frequency) noexcept -> void
{
    _frequency = frequency;
}

template<typename SampleType>
auto Oscillator<SampleType>::setSampleRate(SampleType sampleRate) noexcept -> void
{
    _sampleRate = sampleRate;
}

template<typename SampleType>
auto Oscillator<SampleType>::operator()() noexcept -> SampleType
{
    if (_shape == OscillatorShape::Sine) { return sine(); }
    if (_shape == OscillatorShape::Triangle) { return triangle(); }
    if (_shape == OscillatorShape::Saw) { return saw(); }
    if (_shape == OscillatorShape::Square) { return pulse(_pulseWidth); }
    return 0.0F;
}

template<typename SampleType>
auto Oscillator<SampleType>::sine() noexcept -> SampleType
{
    auto output = std::sin(_phase * numbers::twoPi<SampleType>);

    _phase += 1.0F / (_sampleRate / _frequency);
    _phase -= std::floor(_phase);

    return output;
}

template<typename SampleType>
auto Oscillator<SampleType>::pulse(SampleType width) noexcept -> SampleType
{
    if (width < 0.0F) { width = 0.0F; }
    if (width > 1.0F) { width = 1; }

    if (_phase >= 1.0F) { _phase -= 1.0F; }
    _phase += (1.0F / (_sampleRate / (_frequency)));

    if (_phase < width) { return SampleType{-1}; }
    return 1.0F;
}

template<typename SampleType>
auto Oscillator<SampleType>::saw() noexcept -> SampleType
{
    auto output = _phase;

    if (_phase >= 1.0F) { _phase -= SampleType{2}; }
    _phase += (1.0F / (_sampleRate / (_frequency))) * SampleType{2};

    return output;
}

template<typename SampleType>
auto Oscillator<SampleType>::triangle() noexcept -> SampleType
{
    if (_phase >= 1.0F) { _phase -= 1.0F; }
    _phase += (1.0F / (_sampleRate / (_frequency)));
    if (_phase <= SampleType{0.5}) { return (_phase - SampleType{0.25}) * SampleType{4}; }
    return ((1.0F - _phase) - SampleType{0.25}) * SampleType{4};
}

}  // namespace mc
