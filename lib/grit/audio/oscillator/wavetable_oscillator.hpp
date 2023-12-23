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
struct WavetableOscillator
{
    explicit WavetableOscillator(etl::mdspan<SampleType const, etl::extents<etl::size_t, TableSize>> wavetable);

    auto setPhase(SampleType phase) noexcept -> void;
    auto setFrequency(SampleType frequency) noexcept -> void;
    auto setSampleRate(SampleType sampleRate) noexcept -> void;

    auto addPhaseOffset(SampleType offset) noexcept -> void;

    [[nodiscard]] auto operator()() noexcept -> SampleType;

private:
    SampleType _sampleRate{0};
    SampleType _phase{0};
    SampleType _phaseIncrement{0};
    etl::mdspan<SampleType const, etl::extents<etl::size_t, TableSize>> _wavetable;
};

template<typename SampleType, size_t Size>
[[nodiscard]] constexpr auto makeSineWavetable() noexcept -> etl::array<SampleType, Size>;

template<etl::floating_point SampleType, etl::size_t TableSize>
WavetableOscillator<SampleType, TableSize>::WavetableOscillator(
    etl::mdspan<SampleType const, etl::extents<etl::size_t, TableSize>> wavetable
)
    : _wavetable{wavetable}
{}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto WavetableOscillator<SampleType, TableSize>::setPhase(SampleType phase) noexcept -> void
{
    _phase = phase;
}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto WavetableOscillator<SampleType, TableSize>::setFrequency(SampleType frequency) noexcept -> void
{
    _phaseIncrement = 1.0F / (_sampleRate / frequency);
}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto WavetableOscillator<SampleType, TableSize>::setSampleRate(SampleType sampleRate) noexcept -> void
{
    _sampleRate = sampleRate;
}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto WavetableOscillator<SampleType, TableSize>::addPhaseOffset(SampleType offset) noexcept -> void
{
    _phase += offset;
    _phase -= etl::floor(_phase);
}

template<etl::floating_point SampleType, etl::size_t TableSize>
auto WavetableOscillator<SampleType, TableSize>::operator()() noexcept -> SampleType
{
    if (_wavetable.empty()) {
        return SampleType(0);
    }

    auto const scaledPhase  = _phase * static_cast<SampleType>(_wavetable.size());
    auto const sampleIndex  = static_cast<size_t>(scaledPhase);
    auto const sampleOffset = scaledPhase - static_cast<SampleType>(sampleIndex);
    addPhaseOffset(_phaseIncrement);

    return BufferInterpolation::Hermite{}(_wavetable, sampleIndex, sampleOffset);
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
