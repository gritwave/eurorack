#pragma once

#include <grit/audio/mix/cross_fade.hpp>
#include <grit/audio/oscillator/oscillator.hpp>
#include <grit/math/range.hpp>

#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point Float>
struct variable_shape_oscillator
{
    variable_shape_oscillator() = default;

    auto set_shapes(oscillator_shape a, oscillator_shape b) noexcept -> void;
    auto set_shape_morph(Float morph) noexcept -> void;

    auto set_phase(Float phase) noexcept -> void;
    auto set_frequency(Float frequency) noexcept -> void;
    auto set_sample_rate(Float sample_rate) noexcept -> void;

    auto add_phase_offset(Float offset) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> Float;

private:
    oscillator<Float> _osc_a{};
    oscillator<Float> _osc_b{};
    cross_fade<Float> _cross_fade;
};

template<etl::floating_point Float>
auto variable_shape_oscillator<Float>::set_shapes(oscillator_shape a, oscillator_shape b) noexcept -> void
{
    _osc_a.setShape(a);
    _osc_b.setShape(b);
}

template<etl::floating_point Float>
auto variable_shape_oscillator<Float>::set_shape_morph(Float morph) noexcept -> void
{
    _cross_fade.set_parameter({
        .mix   = etl::clamp(morph, Float{0}, Float{1}),
        .curve = cross_fade_curve::ConstantPower,
    });
}

template<etl::floating_point Float>
auto variable_shape_oscillator<Float>::set_phase(Float phase) noexcept -> void
{
    _osc_a.setPhase(phase);
    _osc_b.setPhase(phase);
}

template<etl::floating_point Float>
auto variable_shape_oscillator<Float>::set_frequency(Float frequency) noexcept -> void
{
    _osc_a.setFrequency(frequency);
    _osc_b.setFrequency(frequency);
}

template<etl::floating_point Float>
auto variable_shape_oscillator<Float>::set_sample_rate(Float sample_rate) noexcept -> void
{
    _osc_a.setSampleRate(sample_rate);
    _osc_b.setSampleRate(sample_rate);
}

template<etl::floating_point Float>
auto variable_shape_oscillator<Float>::add_phase_offset(Float offset) noexcept -> void
{
    _osc_a.addPhaseOffset(offset);
    _osc_b.addPhaseOffset(offset);
}

template<etl::floating_point Float>
auto variable_shape_oscillator<Float>::operator()() noexcept -> Float
{
    return _cross_fade.process(_osc_a(), _osc_b());
}

}  // namespace grit
