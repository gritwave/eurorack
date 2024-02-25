#pragma once

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-unit
template<etl::floating_point Float>
constexpr auto defaultMinusInfinityDb = static_cast<Float>(-100);

/// \ingroup grit-unit
template<etl::floating_point Float>
auto fromDecibels(Float decibels, Float minusInfinityDb = defaultMinusInfinityDb<Float>) -> Float
{
    return decibels > minusInfinityDb ? etl::pow(Float(10.0), decibels * Float(0.05)) : Float();
}

/// \ingroup grit-unit
template<etl::floating_point Float>
auto toDecibels(Float gain, Float minusInfinityDb = defaultMinusInfinityDb<Float>) -> Float
{
    return gain > Float() ? etl::max(minusInfinityDb, static_cast<Float>(etl::log10(gain)) * Float(20.0))
                          : minusInfinityDb;
}

/// \ingroup grit-unit
template<etl::floating_point Float>
struct Decibels
{
    using ValueType = Float;

    constexpr Decibels() = default;

    explicit constexpr Decibels(Float dB) : _dB{dB} {}

    [[nodiscard]] constexpr explicit operator Float() const { return _dB; }

    [[nodiscard]] constexpr auto value() const -> Float { return _dB; }

    [[nodiscard]] constexpr auto toGain() const -> Float { return fromDecibels(_dB); }

    [[nodiscard]] static constexpr auto fromGain(Float gain) -> Decibels { return Decibels{toDecibels(gain)}; }

    friend constexpr auto operator+(Decibels x) -> Decibels { return x; }

    friend constexpr auto operator-(Decibels x) -> Decibels { return Decibels{-x.value()}; }

    friend constexpr auto operator+(Decibels lhs, Decibels rhs) -> Decibels
    {
        return Decibels{lhs.value() + rhs.value()};
    }

    friend constexpr auto operator-(Decibels lhs, Decibels rhs) -> Decibels
    {
        return Decibels{lhs.value() - rhs.value()};
    }

    friend constexpr auto operator*(Decibels lhs, Float rhs) -> Decibels { return Decibels{lhs.value() * rhs}; }

    friend constexpr auto operator/(Decibels lhs, Float rhs) -> Decibels { return Decibels{lhs.value() / rhs}; }

    friend constexpr auto operator*(Float lhs, Decibels rhs) -> Decibels { return Decibels{rhs.value() * lhs}; }

    friend constexpr auto operator/(Float lhs, Decibels rhs) -> Decibels { return Decibels{rhs.value() / lhs}; }

    friend constexpr auto operator==(Decibels lhs, Decibels rhs) -> bool { return lhs.value() == rhs.value(); }

    friend constexpr auto operator!=(Decibels lhs, Decibels rhs) -> bool { return lhs.value() != rhs.value(); }

    friend constexpr auto operator<(Decibels lhs, Decibels rhs) -> bool { return lhs.value() < rhs.value(); }

    friend constexpr auto operator<=(Decibels lhs, Decibels rhs) -> bool { return lhs.value() <= rhs.value(); }

    friend constexpr auto operator>(Decibels lhs, Decibels rhs) -> bool { return lhs.value() > rhs.value(); }

    friend constexpr auto operator>=(Decibels lhs, Decibels rhs) -> bool { return lhs.value() >= rhs.value(); }

private:
    Float _dB{0};
};

}  // namespace grit
