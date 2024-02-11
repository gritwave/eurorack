#pragma once

#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/audio/waveshape/wave_shaper_adaa1.hpp>

namespace grit {

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
struct HalfWaveRectifierNonlinearity
{
    constexpr HalfWaveRectifierNonlinearity() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) { return x > Float(0) ? x : Float(0); }

    [[nodiscard]] static constexpr auto ad1(Float x) { return x <= Float(0) ? Float(0) : x * x * 0.5; }

    [[nodiscard]] static constexpr auto ad2(Float x)
    {
        return x <= Float(0) ? Float(0) : x * x * x * (Float(1) / Float(6));
    }
};

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using HalfWaveRectifier = WaveShaper<Float, HalfWaveRectifierNonlinearity<Float>>;

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using HalfWaveRectifierADAA1 = WaveShaperADAA1<Float, HalfWaveRectifierNonlinearity<Float>>;

}  // namespace grit
