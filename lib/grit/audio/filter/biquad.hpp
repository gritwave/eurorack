#pragma once

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/span.hpp>

namespace grit {

/// Calculates coefficents for 2nd order biquad filters
/// \see Biquad
/// \ingroup grit-audio-filter
template<etl::floating_point Float>
struct BiquadCoefficients
{
    using SampleType = Float;

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

    [[nodiscard]] static constexpr auto makeBypass() -> etl::array<Float, 6>;

    /// Creates coefficients for a second order low-pass filter.
    /// Formulas are given in chapter 6.6.2 of \cite Pirkle2012
    [[nodiscard]] static constexpr auto makeLowPass(Float cutoff, Float Q, Float sampleRate) -> etl::array<Float, 6>;

    /// Creates coefficients for a second order high-pass filter.
    /// Formulas are given in chapter 6.6.2 of \cite Pirkle2012
    [[nodiscard]] static constexpr auto makeHighPass(Float cutoff, Float Q, Float sampleRate) -> etl::array<Float, 6>;
};

/// \brief 2nd order IIR filter using the transpose direct form 2 structure.
/// \ingroup grit-audio-filter
template<etl::floating_point Float>
struct Biquad
{
    using SampleType   = Float;
    using Coefficients = BiquadCoefficients<Float>;

    constexpr Biquad() = default;

    constexpr auto setCoefficients(etl::span<Float const, 6> coefficients) -> void;
    constexpr auto reset() -> void;

    [[nodiscard]] constexpr auto operator()(Float x) -> Float;

private:
    using Index = Coefficients::Index;
    etl::array<Float, Index::NumCoefficients> _coefficients{Coefficients::makeBypass()};
    etl::array<Float, 2> _z{};
};

template<etl::floating_point Float>
constexpr auto BiquadCoefficients<Float>::makeBypass() -> etl::array<Float, 6>
{
    return {Float(1), Float(0), Float(0), Float(1), Float(0), Float(0)};
}

template<etl::floating_point Float>
constexpr auto BiquadCoefficients<Float>::makeLowPass(Float cutoff, Float Q, Float sampleRate) -> etl::array<Float, 6>
{
    auto const omega0 = Float(2) * static_cast<Float>(etl::numbers::pi) * cutoff / sampleRate;
    auto const d      = Float(1) / Q;
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

template<etl::floating_point Float>
constexpr auto BiquadCoefficients<Float>::makeHighPass(Float cutoff, Float Q, Float sampleRate) -> etl::array<Float, 6>
{
    auto const omega0 = Float(2) * static_cast<Float>(etl::numbers::pi) * cutoff / sampleRate;
    auto const d      = Float(1) / Q;
    auto const cos0   = etl::cos(omega0);
    auto const sin0   = etl::sin(omega0);
    auto const beta   = Float(0.5) * ((Float(1) - (d * Float(0.5)) * sin0) / (Float(1) + (d * Float(0.5)) * sin0));
    auto const gamma  = (Float(0.5) + beta) * cos0;

    auto const b0 = (Float(0.5) + beta + gamma) * Float(0.5);
    auto const b1 = -(Float(0.5) + beta + gamma);
    auto const b2 = b0;

    auto const a0 = Float(1);
    auto const a1 = Float(-2) * gamma;
    auto const a2 = Float(2) * beta;

    return {b0, b1, b2, a0, a1, a2};
}

template<etl::floating_point Float>
constexpr auto Biquad<Float>::setCoefficients(etl::span<Float const, 6> coefficients) -> void
{
    etl::copy(coefficients.begin(), coefficients.end(), _coefficients.begin());
}

template<etl::floating_point Float>
constexpr auto Biquad<Float>::reset() -> void
{
    _z[0] = Float(0);
    _z[1] = Float(0);
}

template<etl::floating_point Float>
constexpr auto Biquad<Float>::operator()(Float x) -> Float
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

}  // namespace grit
