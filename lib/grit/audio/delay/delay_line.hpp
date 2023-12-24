#pragma once

#include <grit/math/buffer_interpolation.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/mdspan.hpp>

namespace grit {

template<etl::floating_point Float, typename Interpolation = buffer_interpolation::hermite>
struct delay_line
{
    explicit delay_line(etl::mdspan<Float, etl::dextents<etl::size_t, 1>> buffer) noexcept;

    auto set_delay(Float delay_in_samples) -> void;

    auto push_sample(Float sample) -> void;
    auto pop_sample() -> Float;

    auto reset() -> void;

private:
    Float _frac{0};
    etl::size_t _delay{0};
    etl::size_t _write_pos{0};
    etl::mdspan<Float, etl::dextents<etl::size_t, 1>> _buffer;
    [[no_unique_address]] Interpolation _interpolator{};
};

template<etl::floating_point Float, typename Interpolation>
delay_line<Float, Interpolation>::delay_line(etl::mdspan<Float, etl::dextents<etl::size_t, 1>> buffer) noexcept
    : _buffer{buffer}
{}

template<etl::floating_point Float, typename Interpolation>
auto delay_line<Float, Interpolation>::set_delay(Float delay_in_samples) -> void
{
    auto const delay = static_cast<etl::size_t>(delay_in_samples);
    _frac            = delay_in_samples - static_cast<Float>(delay);
    _delay           = etl::clamp<etl::size_t>(delay, 0, _buffer.extent(0) - 1);
}

template<etl::floating_point Float, typename Interpolation>
auto delay_line<Float, Interpolation>::push_sample(Float sample) -> void
{
    _buffer(_write_pos) = sample;
    _write_pos          = (_write_pos - 1 + _buffer.extent(0)) % _buffer.extent(0);
}

template<etl::floating_point Float, typename Interpolation>
auto delay_line<Float, Interpolation>::pop_sample() -> Float
{
    auto const read_pos = _write_pos + _delay;
    return _interpolator(_buffer, read_pos, _frac);
}

template<etl::floating_point Float, typename Interpolation>
auto delay_line<Float, Interpolation>::reset() -> void
{
    _write_pos = 0;
    for (auto i{0}; etl::cmp_less(i, _buffer.extent(0)); ++i) {
        _buffer(i) = Float(0);
    }
}

}  // namespace grit
