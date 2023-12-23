#pragma once

#include <grit/math/buffer_interpolation.hpp>
#include <grit/math/range.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/mdspan.hpp>
#include <etl/numbers.hpp>

namespace grit {

template<etl::floating_point SampleType, etl::size_t TableSize = etl::dynamic_extent>
struct wavetable_oscillator
{
    explicit wavetable_oscillator(etl::mdspan<SampleType const, etl::extents<etl::size_t, TableSize>> wavetable);

    auto set_phase(SampleType phase) noexcept -> void;
    auto set_frequency(SampleType frequency) noexcept -> void;
    auto set_sample_rate(SampleType sampleRate) noexcept -> void;

    auto add_phase_offset(SampleType offset) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> SampleType;

private:
    SampleType _sample_rate{0};
    SampleType _phase{0};
    SampleType _phase_increment{0};
    etl::mdspan<SampleType const, etl::extents<etl::size_t, TableSize>> _wavetable;
};

template<typename SampleType, size_t Size>
[[nodiscard]] constexpr auto makeSineWavetable() noexcept -> etl::array<SampleType, Size>;

template<etl::floating_point SampleType, etl::size_t TableSize>
wavetable_oscillator<SampleType, TableSize>::wavetable_oscillator(
    etl::mdspan<SampleType const, etl::extents<etl::size_t, TableSize>> wavetable
)
    : _wavetable{wavetable}
{}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto wavetable_oscillator<SampleType, TableSize>::set_phase(SampleType phase) noexcept -> void
{
    _phase = phase;
}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto wavetable_oscillator<SampleType, TableSize>::set_frequency(SampleType frequency) noexcept -> void
{
    _phase_increment = 1.0F / (_sample_rate / frequency);
}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto wavetable_oscillator<SampleType, TableSize>::set_sample_rate(SampleType sampleRate) noexcept -> void
{
    _sample_rate = sampleRate;
}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto wavetable_oscillator<SampleType, TableSize>::add_phase_offset(SampleType offset) noexcept -> void
{
    _phase += offset;
    _phase -= etl::floor(_phase);
}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto wavetable_oscillator<SampleType, TableSize>::operator()() noexcept -> SampleType
{
    if (_wavetable.empty()) {
        return SampleType(0);
    }

    auto const scaled_phase  = _phase * static_cast<SampleType>(_wavetable.size());
    auto const sample_index  = static_cast<size_t>(scaled_phase);
    auto const sample_offset = scaled_phase - static_cast<SampleType>(sample_index);
    add_phase_offset(_phase_increment);

    return buffer_interpolation::hermite{}(_wavetable, sample_index, sample_offset);
}

template<typename SampleType, size_t Size>
constexpr auto makeSineWavetable() noexcept -> etl::array<SampleType, Size>
{
    auto const delta = static_cast<SampleType>(etl::numbers::pi * 2.0) / static_cast<SampleType>(Size - 1U);

    auto table = etl::array<SampleType, Size>{};
    auto phase = SampleType(0);
    auto gen   = [&] {
        auto const value = etl::sin(phase);
        phase += delta;
        return value;
    };

    etl::generate(begin(table), end(table), gen);
    return table;
}
}  // namespace grit
