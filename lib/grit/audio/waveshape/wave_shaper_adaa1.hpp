#pragma once

#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-audio-waveshape
template<etl::floating_point Float, typename Nonlinearity>
struct WaveShaperADAA1
{
    using value_type = Float;

    constexpr WaveShaperADAA1() = default;

    constexpr auto reset() -> void
    {
        _xm1   = Float(0);
        _ad1m1 = Float(0);
    }

    [[nodiscard]] constexpr auto operator()(Float x) -> Float
    {
        auto const tooSmall = etl::abs(x - _xm1) < tolerance;
        auto const ad1      = _nl.ad1(x);
        auto const y        = tooSmall ? _nl.f((x + _xm1) * Float(0.5)) : (ad1 - _ad1m1) / (x - _xm1);

        _xm1   = x;
        _ad1m1 = ad1;

        return y;
    }

private:
    static constexpr auto const tolerance = Float(1e-3);

    Float _xm1{0};
    Float _ad1m1{0};
    TETL_NO_UNIQUE_ADDRESS Nonlinearity _nl;
};

}  // namespace grit
