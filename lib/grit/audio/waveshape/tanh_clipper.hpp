#pragma once

#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/audio/waveshape/wave_shaper_adaa1.hpp>

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
struct TanhClipperNonlinearity
{
    constexpr TanhClipperNonlinearity() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const -> Float { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) -> Float { return etl::tanh(x); }

    [[nodiscard]] static constexpr auto ad1(Float x) -> Float { return x - etl::log(etl::tanh(x) + 1); }
};

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using TanhClipper = WaveShaper<Float, TanhClipperNonlinearity<Float>>;

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using TanhClipperADAA1 = WaveShaperADAA1<Float, TanhClipperNonlinearity<Float>>;

}  // namespace grit
