#pragma once

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/span.hpp>

namespace grit {

/// Calculates coefficents for 2nd order biquad filters
/// \see BiquadTDF2
/// \ingroup grit-audio-filter
template<etl::floating_point Float>
struct BiquadCoefficients
{
    using value_type = Float;
    using SampleType = Float;

    [[nodiscard]] static constexpr auto makeBypass() -> etl::array<Float, 6>
    {
        return {Float(1), Float(0), Float(0), Float(1), Float(0), Float(0)};
    }

    [[nodiscard]] static constexpr auto makeLowPass(Float f0, Float q, Float fs) -> etl::array<Float, 6>
    {
        auto const omega0 = Float(2) * static_cast<Float>(etl::numbers::pi) * f0 / fs;
        auto const d      = Float(1) / q;
        auto const cos0   = etl::cos(omega0);
        auto const sin0   = etl::sin(omega0);
        auto const beta   = Float(0.5) * ((Float(1) - (d * Float(0.5)) * sin0) / (Float(1) + (d * Float(0.5)) * sin0));
        auto const gamma  = (Float(0.5) + beta) * cos0;

        auto const b0 = (Float(0.5) + beta - gamma) * Float(0.5);
        auto const b1 = Float(0.5) + beta - gamma;
        auto const b2 = b0;

        auto const a0 = Float(1);
        auto const a1 = Float(-2) * gamma;
        auto const a2 = Float(2) * beta;

        return {b0, b1, b2, a0, a1, a2};
    }
};

/// \brief 2nd order IIR filter using the transpose direct form 2 structure.
/// \ingroup grit-audio-filter
template<etl::floating_point Float>
struct BiquadTDF2
{
    using value_type   = Float;
    using SampleType   = Float;
    using Coefficients = BiquadCoefficients<Float>;

    constexpr BiquadTDF2() = default;

    constexpr auto setCoefficients(etl::span<Float const, 6> coefficients) -> void
    {
        etl::copy(coefficients.begin(), coefficients.end(), _coefficients.begin());
    }

    constexpr auto reset() -> void
    {
        _z[0] = Float(0);
        _z[1] = Float(0);
    }

    [[nodiscard]] constexpr auto operator()(Float x) -> Float
    {
        auto const b0 = _coefficients[Index::B0];
        auto const b1 = _coefficients[Index::B1];
        auto const b2 = _coefficients[Index::B2];
        auto const a1 = _coefficients[Index::A1];
        auto const a2 = _coefficients[Index::A2];

        auto const y = b0 * x + _z[0];
        _z[0]        = b1 * x - a1 * y + _z[1];
        _z[1]        = b2 * x - a2 * y;
        return y;
    }

private:
    enum Index
    {
        B0,
        B1,
        B2,
        A0,
        A1,
        A2,
        NumCoefficients,
    };

    etl::array<Float, NumCoefficients> _coefficients{Coefficients::makeBypass()};
    etl::array<Float, 2> _z{};
};

}  // namespace grit
