#pragma once

#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/audio/waveshape/wave_shaper_adaa1.hpp>
#include <grit/math/sign.hpp>

namespace grit {

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
struct FullWaveRectifierNonlinearity
{
    constexpr FullWaveRectifierNonlinearity() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) { return etl::abs(x); }

    [[nodiscard]] static constexpr auto ad1(Float x) { return x * x * Float(0.5) * sign(x); }

    [[nodiscard]] static constexpr auto ad2(Float x) { return x * x * x * (Float(1) / Float(6)) * sign(x); }
};

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using FullWaveRectifier = WaveShaper<Float, FullWaveRectifierNonlinearity<Float>>;

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using FullWaveRectifierADAA1 = WaveShaperADAA1<Float, FullWaveRectifierNonlinearity<Float>>;

}  // namespace grit
