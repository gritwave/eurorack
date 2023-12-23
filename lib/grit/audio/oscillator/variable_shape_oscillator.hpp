#pragma once

#include <grit/audio/mix/cross_fade.hpp>
#include <grit/audio/oscillator/oscillator.hpp>
#include <grit/math/range.hpp>

#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point SampleType>
struct variable_shape_oscillator
{
    variable_shape_oscillator() = default;

    auto set_shapes(oscillator_shape a, oscillator_shape b) noexcept -> void;
    auto set_shape_morph(SampleType morph) noexcept -> void;

    auto set_phase(SampleType phase) noexcept -> void;
    auto set_frequency(SampleType frequency) noexcept -> void;
    auto set_sample_rate(SampleType sample_rate) noexcept -> void;

    auto add_phase_offset(SampleType offset) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> SampleType;

private:
    oscillator<SampleType> _osc_a{};
    oscillator<SampleType> _osc_b{};
    cross_fade<SampleType> _cross_fade;
};

template<etl::floating_point SampleType>
auto variable_shape_oscillator<SampleType>::set_shapes(oscillator_shape a, oscillator_shape b) noexcept -> void
{
    _osc_a.setShape(a);
    _osc_b.setShape(b);
}

template<etl::floating_point SampleType>
auto variable_shape_oscillator<SampleType>::set_shape_morph(SampleType morph) noexcept -> void
{
    _cross_fade.set_parameter({
        .mix   = etl::clamp(morph, SampleType{0}, SampleType{1}),
        .curve = cross_fade_curve::ConstantPower,
    });
}

template<etl::floating_point SampleType>
auto variable_shape_oscillator<SampleType>::set_phase(SampleType phase) noexcept -> void
{
    _osc_a.setPhase(phase);
    _osc_b.setPhase(phase);
}

template<etl::floating_point SampleType>
auto variable_shape_oscillator<SampleType>::set_frequency(SampleType frequency) noexcept -> void
{
    _osc_a.setFrequency(frequency);
    _osc_b.setFrequency(frequency);
}

template<etl::floating_point SampleType>
auto variable_shape_oscillator<SampleType>::set_sample_rate(SampleType sample_rate) noexcept -> void
{
    _osc_a.setSampleRate(sample_rate);
    _osc_b.setSampleRate(sample_rate);
}

template<etl::floating_point SampleType>
auto variable_shape_oscillator<SampleType>::add_phase_offset(SampleType offset) noexcept -> void
{
    _osc_a.addPhaseOffset(offset);
    _osc_b.addPhaseOffset(offset);
}

template<etl::floating_point SampleType>
auto variable_shape_oscillator<SampleType>::operator()() noexcept -> SampleType
{
    return _cross_fade.process(_osc_a(), _osc_b());
}

}  // namespace grit
