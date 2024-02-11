#pragma once

#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/audio/waveshape/wave_shaper_adaa1.hpp>

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
struct DiodeRectifierNonlinearity
{
    constexpr DiodeRectifierNonlinearity() = default;

    [[nodiscard]] constexpr auto operator()(Float x) const -> Float { return f(x); }

    [[nodiscard]] static constexpr auto f(Float x) -> Float
    {
        return Float(0.2) * etl::exp(Float(1.79) * x) - Float(0.2);
    }

    [[nodiscard]] static constexpr auto ad1(Float x) -> Float
    {
        return Float(-0.2) * x + Float(0.111731843575419) * etl::exp(Float(1.79) * x);
    }
};

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using DiodeRectifier = WaveShaper<Float, DiodeRectifierNonlinearity<Float>>;

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float>
using DiodeRectifierADAA1 = WaveShaperADAA1<Float, DiodeRectifierNonlinearity<Float>>;

}  // namespace grit
