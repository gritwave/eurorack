#pragma once

#include <grit/audio/delay/non_owning_delay_line.hpp>
#include <grit/math/buffer_interpolation.hpp>

#include <etl/array.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-audio-delay
template<etl::floating_point Float, etl::size_t MaxDelay, typename Interpolation = BufferInterpolation::Hermite>
struct StaticDelayLine
{
    StaticDelayLine() = default;

    auto setDelay(Float delayInSamples) -> void;

    auto pushSample(Float sample) -> void;
    auto popSample() -> Float;

    auto reset() -> void;

private:
    etl::array<Float, MaxDelay> _buffer{};
    NonOwningDelayLine<Float, etl::extents<etl::size_t, MaxDelay>, Interpolation> _delayLine{
        etl::mdspan(_buffer.data(), etl::extents<etl::size_t, MaxDelay>{})
    };
};

template<etl::floating_point Float, etl::size_t MaxDelay, typename Interpolation>
auto StaticDelayLine<Float, MaxDelay, Interpolation>::setDelay(Float delayInSamples) -> void
{
    _delayLine.setDelay(delayInSamples);
}

template<etl::floating_point Float, etl::size_t MaxDelay, typename Interpolation>
auto StaticDelayLine<Float, MaxDelay, Interpolation>::pushSample(Float sample) -> void
{
    _delayLine.pushSample(sample);
}

template<etl::floating_point Float, etl::size_t MaxDelay, typename Interpolation>
auto StaticDelayLine<Float, MaxDelay, Interpolation>::popSample() -> Float
{
    return _delayLine.popSample();
}

template<etl::floating_point Float, etl::size_t MaxDelay, typename Interpolation>
auto StaticDelayLine<Float, MaxDelay, Interpolation>::reset() -> void
{
    _delayLine.reset();
}

}  // namespace grit
