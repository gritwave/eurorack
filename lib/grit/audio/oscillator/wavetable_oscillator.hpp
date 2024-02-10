#pragma once

#include <grit/math/buffer_interpolation.hpp>
#include <grit/math/remap.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/mdspan.hpp>
#include <etl/numbers.hpp>

namespace grit {

template<etl::floating_point Float, etl::size_t TableSize = etl::dynamic_extent>
struct WavetableOscillator
{
    explicit WavetableOscillator(etl::mdspan<Float const, etl::extents<etl::size_t, TableSize>> wavetable);

    auto setPhase(Float phase) -> void;
    auto setFrequency(Float frequency) -> void;
    auto setSampleRate(Float sampleRate) -> void;

    auto addPhaseOffset(Float offset) -> void;

    [[nodiscard]] auto operator()() -> Float;

private:
    Float _sampleRate{0};
    Float _phase{0};
    Float _phaseIncrement{0};
    etl::mdspan<Float const, etl::extents<etl::size_t, TableSize>> _wavetable;
};

template<typename Float, etl::size_t Size>
[[nodiscard]] constexpr auto makeSineWavetable() -> etl::array<Float, Size>;

template<etl::floating_point Float, etl::size_t TableSize>
WavetableOscillator<Float, TableSize>::WavetableOscillator(
    etl::mdspan<Float const, etl::extents<etl::size_t, TableSize>> wavetable
)
    : _wavetable{wavetable}
{}

template<etl::floating_point Float, etl::size_t TableSize>
auto WavetableOscillator<Float, TableSize>::setPhase(Float phase) -> void
{
    _phase = phase;
}

template<etl::floating_point Float, etl::size_t TableSize>
auto WavetableOscillator<Float, TableSize>::setFrequency(Float frequency) -> void
{
    _phaseIncrement = 1.0F / (_sampleRate / frequency);
}

template<etl::floating_point Float, etl::size_t TableSize>
auto WavetableOscillator<Float, TableSize>::setSampleRate(Float sampleRate) -> void
{
    _sampleRate = sampleRate;
}

template<etl::floating_point Float, etl::size_t TableSize>
auto WavetableOscillator<Float, TableSize>::addPhaseOffset(Float offset) -> void
{
    _phase += offset;
    _phase -= etl::floor(_phase);
}

template<etl::floating_point Float, etl::size_t TableSize>
auto WavetableOscillator<Float, TableSize>::operator()() -> Float
{
    if (_wavetable.empty()) {
        return Float(0);
    }

    auto const scaledPhase  = _phase * static_cast<Float>(_wavetable.size());
    auto const sampleIndex  = static_cast<etl::size_t>(scaledPhase);
    auto const sampleOffset = scaledPhase - static_cast<Float>(sampleIndex);
    addPhaseOffset(_phaseIncrement);

    return BufferInterpolation::Hermite{}(_wavetable, sampleIndex, sampleOffset);
}

template<typename Float, etl::size_t Size>
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
