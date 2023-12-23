#pragma once

#include <grit/audio/delay/delay_line.hpp>
#include <grit/math/buffer_interpolation.hpp>

#include <etl/array.hpp>
#include <etl/concepts.hpp>

namespace grit {

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation = buffer_interpolation::hermite>
struct static_delay_line
{
    static_delay_line() = default;

    auto set_delay(SampleType delay_in_samples) -> void;

    auto push_sample(SampleType sample) -> void;
    auto pop_sample() -> SampleType;

    auto reset() -> void;

private:
    etl::array<SampleType, MaxDelay> _buffer{};
    delay_line<SampleType, Interpolation> _delay_line{
        etl::mdspan{_buffer.data(), etl::dextents<etl::size_t, 1>{MaxDelay}}
    };
};

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation>
auto static_delay_line<SampleType, MaxDelay, Interpolation>::set_delay(SampleType delay_in_samples) -> void
{
    _delay_line.set_delay(delay_in_samples);
}

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation>
auto static_delay_line<SampleType, MaxDelay, Interpolation>::push_sample(SampleType sample) -> void
{
    _delay_line.push_sample(sample);
}

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation>
auto static_delay_line<SampleType, MaxDelay, Interpolation>::pop_sample() -> SampleType
{
    return _delay_line.pop_sample();
}

template<etl::floating_point SampleType, etl::size_t MaxDelay, typename Interpolation>
auto static_delay_line<SampleType, MaxDelay, Interpolation>::reset() -> void
{
    _delay_line.reset();
}

}  // namespace grit
