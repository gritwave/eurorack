#pragma once

#include <grit/math/range.hpp>

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>

namespace grit {

enum struct oscillator_shape
{
    Sine,
    Triangle,
    Square,
};

template<etl::floating_point SampleType>
struct oscillator
{
    oscillator() = default;

    auto set_shape(oscillator_shape shape) noexcept -> void;
    auto set_phase(SampleType phase) noexcept -> void;
    auto set_frequency(SampleType frequency) noexcept -> void;
    auto set_sample_rate(SampleType sample_rate) noexcept -> void;

    auto add_phase_offset(SampleType offset) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> SampleType;

private:
    [[nodiscard]] static auto sine(SampleType phase) noexcept -> SampleType;
    [[nodiscard]] static auto triangle(SampleType phase) noexcept -> SampleType;
    [[nodiscard]] static auto pulse(SampleType phase, SampleType width) noexcept -> SampleType;

    oscillator_shape _shape{oscillator_shape::Sine};
    SampleType _sample_rate{0};
    SampleType _phase{0};
    SampleType _phase_increment{0};
    SampleType _pulse_width{0.5};
};

template<etl::floating_point SampleType>
auto oscillator<SampleType>::set_shape(oscillator_shape shape) noexcept -> void
{
    _shape = shape;
}

template<etl::floating_point SampleType>
auto oscillator<SampleType>::set_phase(SampleType phase) noexcept -> void
{
    _phase = phase;
}

template<etl::floating_point SampleType>
auto oscillator<SampleType>::set_frequency(SampleType frequency) noexcept -> void
{
    _phase_increment = 1.0F / (_sample_rate / frequency);
}

template<etl::floating_point SampleType>
auto oscillator<SampleType>::set_sample_rate(SampleType sample_rate) noexcept -> void
{
    _sample_rate = sample_rate;
}

template<etl::floating_point SampleType>
auto oscillator<SampleType>::add_phase_offset(SampleType offset) noexcept -> void
{
    _phase += offset;
    _phase -= etl::floor(_phase);
}

template<etl::floating_point SampleType>
auto oscillator<SampleType>::operator()() noexcept -> SampleType
{
    auto output = SampleType{};
    switch (_shape) {
        case oscillator_shape::Sine: {
            output = sine(_phase);
            break;
        }
        case oscillator_shape::Triangle: {
            output = triangle(_phase);
            break;
        }
        case oscillator_shape::Square: {
            output = pulse(_phase, _pulse_width);
            break;
        }
        default: {
            break;
        }
    }

    add_phase_offset(_phase_increment);
    return output;
}

template<etl::floating_point SampleType>
auto oscillator<SampleType>::sine(SampleType phase) noexcept -> SampleType
{
    static constexpr auto two_pi = static_cast<SampleType>(etl::numbers::pi) * SampleType{2};
    return etl::sin(phase * two_pi);
}

template<etl::floating_point SampleType>
auto oscillator<SampleType>::triangle(SampleType phase) noexcept -> SampleType
{
    auto const x = phase <= SampleType{0.5} ? phase : SampleType{1} - phase;
    return (x - SampleType{0.25}) * SampleType{4};
}

template<etl::floating_point SampleType>
auto oscillator<SampleType>::pulse(SampleType phase, SampleType width) noexcept -> SampleType
{
    auto const w = etl::clamp(width, SampleType{0}, SampleType{1});
    if (phase < w) {
        return SampleType{-1};
    }
    return SampleType{1};
}

}  // namespace grit
