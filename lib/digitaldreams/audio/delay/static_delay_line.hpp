#pragma once

#include <digitaldreams/audio/delay/delay_interpolation.hpp>
#include <digitaldreams/audio/math/hermite_interpolation.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/cmath.hpp>

namespace digitaldreams::audio
{

template<typename SampleType, etl::size_t MaxSize, DelayInterpolation Interpolation = DelayInterpolation::Hermite4>
struct StaticDelayLine
{
    StaticDelayLine() = default;

    auto setDelay(SampleType delayInSamples) -> void;

    auto pushSample(SampleType sample) -> void;
    auto popSample() -> SampleType;

    auto reset() -> void;

private:
    SampleType _frac{0};
    etl::size_t _delay{0};
    etl::size_t _writePos{0};
    etl::array<SampleType, MaxSize> _buffer;
};

template<typename SampleType, etl::size_t MaxSize, DelayInterpolation Interpolation>
auto StaticDelayLine<SampleType, MaxSize, Interpolation>::setDelay(SampleType delayInSamples) -> void
{
    auto const delay = static_cast<etl::size_t>(delayInSamples);
    _frac            = delayInSamples - static_cast<SampleType>(delay);
    _delay           = etl::clamp<etl::size_t>(delay, 0, MaxSize - 1);
}

template<typename SampleType, etl::size_t MaxSize, DelayInterpolation Interpolation>
auto StaticDelayLine<SampleType, MaxSize, Interpolation>::pushSample(SampleType sample) -> void
{
    _buffer[_writePos] = sample;
    _writePos          = (_writePos - 1 + MaxSize) % MaxSize;
}

template<typename SampleType, etl::size_t MaxSize, DelayInterpolation Interpolation>
auto StaticDelayLine<SampleType, MaxSize, Interpolation>::popSample() -> SampleType
{
    if constexpr (Interpolation == DelayInterpolation::None)
    {
        auto const readPos = _writePos + _delay;
        return _buffer[readPos % MaxSize];
    }
    else if constexpr (Interpolation == DelayInterpolation::Linear)
    {
        auto const readPos = _writePos + _delay;
        auto const x0      = _buffer[readPos % MaxSize];
        auto const x1      = _buffer[(readPos + 1) % MaxSize];
        return etl::lerp(x0, x1, _frac);
    }
    else
    {
        auto const readPos = static_cast<etl::ptrdiff_t>(_writePos + _delay);
        auto const xm1     = _buffer[(readPos - 1) % MaxSize];
        auto const x0      = _buffer[readPos % MaxSize];
        auto const x1      = _buffer[(readPos + 1) % MaxSize];
        auto const x2      = _buffer[(readPos + 2) % MaxSize];
        return hermite4(_frac, xm1, x0, x1, x2);
    }
}

template<typename SampleType, etl::size_t MaxSize, DelayInterpolation Interpolation>
auto StaticDelayLine<SampleType, MaxSize, Interpolation>::reset() -> void
{
    _writePos = 0;
    etl::fill(_buffer.begin(), _buffer.end(), SampleType{0});
}

}  // namespace digitaldreams::audio
