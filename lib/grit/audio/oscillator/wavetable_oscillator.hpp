#pragma once

#include <grit/math/buffer_interpolation.hpp>
#include <grit/math/range.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/mdspan.hpp>
#include <etl/numbers.hpp>

namespace grit {

template<etl::floating_point Float, etl::size_t TableSize = etl::dynamic_extent>
struct wavetable_oscillator
{
    explicit wavetable_oscillator(etl::mdspan<Float const, etl::extents<etl::size_t, TableSize>> wavetable);

    auto set_phase(Float phase) -> void;
    auto set_frequency(Float frequency) -> void;
    auto set_sample_rate(Float sampleRate) -> void;

    auto add_phase_offset(Float offset) -> void;

    [[nodiscard]] auto operator()() -> Float;

private:
    Float _sample_rate{0};
    Float _phase{0};
    Float _phase_increment{0};
    etl::mdspan<Float const, etl::extents<etl::size_t, TableSize>> _wavetable;
};

template<typename Float, size_t Size>
[[nodiscard]] constexpr auto makeSineWavetable() -> etl::array<Float, Size>;

template<etl::floating_point Float, etl::size_t TableSize>
wavetable_oscillator<Float, TableSize>::wavetable_oscillator(
    etl::mdspan<Float const, etl::extents<etl::size_t, TableSize>> wavetable
)
    : _wavetable{wavetable}
{}

template<etl::floating_point Float, etl::size_t TableSize>
auto wavetable_oscillator<Float, TableSize>::set_phase(Float phase) -> void
{
    _phase = phase;
}

template<etl::floating_point Float, etl::size_t TableSize>
auto wavetable_oscillator<Float, TableSize>::set_frequency(Float frequency) -> void
{
    _phase_increment = 1.0F / (_sample_rate / frequency);
}

template<etl::floating_point Float, etl::size_t TableSize>
auto wavetable_oscillator<Float, TableSize>::set_sample_rate(Float sampleRate) -> void
{
    _sample_rate = sampleRate;
}

template<etl::floating_point Float, etl::size_t TableSize>
auto wavetable_oscillator<Float, TableSize>::add_phase_offset(Float offset) -> void
{
    _phase += offset;
    _phase -= etl::floor(_phase);
}

template<etl::floating_point Float, etl::size_t TableSize>
auto wavetable_oscillator<Float, TableSize>::operator()() -> Float
{
    if (_wavetable.empty()) {
        return Float(0);
    }

    auto const scaled_phase  = _phase * static_cast<Float>(_wavetable.size());
    auto const sample_index  = static_cast<size_t>(scaled_phase);
    auto const sample_offset = scaled_phase - static_cast<Float>(sample_index);
    add_phase_offset(_phase_increment);

    return BufferInterpolation::Hermite{}(_wavetable, sample_index, sample_offset);
}

template<typename Float, size_t Size>
constexpr auto makeSineWavetable() -> etl::array<Float, Size>
{
    auto const delta = static_cast<Float>(etl::numbers::pi * 2.0) / static_cast<Float>(Size - 1U);

    auto table = etl::array<Float, Size>{};
    auto phase = Float(0);
    auto gen   = [&] {
        auto const value = etl::sin(phase);
        phase += delta;
        return value;
    };

    etl::generate(begin(table), end(table), gen);
    return table;
}
}  // namespace grit
