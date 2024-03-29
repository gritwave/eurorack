#pragma once

#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/audio/waveshape/wave_shaper_adaa1.hpp>
#include <grit/math/sign.hpp>

#include <etl/algorithm.hpp>

namespace grit {

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
struct HardClipperNonlinearity
{
    constexpr HardClipperNonlinearity() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) { return etl::clamp(x, Float(-1), Float(+1)); }

    [[nodiscard]] static constexpr auto ad1(Float x)
    {
        return etl::abs(x) > Float(1) ? x * sign(x) : (x * x) * Float(0.5);
    }
};

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using HardClipper = WaveShaper<Float, HardClipperNonlinearity<Float>>;

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using HardClipperADAA1 = WaveShaperADAA1<Float, HardClipperNonlinearity<Float>>;

}  // namespace grit
