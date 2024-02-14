#pragma once

#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-math
template<int Exponent, typename Numeric>
    requires(Exponent >= 0)
[[nodiscard]] constexpr auto power(Numeric base) -> Numeric
{
    // Unrolling is necessary when not compiling with `-ffast-math`.
    // https://godbolt.org/z/bEnvWKdsq

    if constexpr (Exponent == 0) {
        return Numeric(1);
    } else if constexpr (Exponent == 1) {
        return base;
    } else if constexpr (Exponent == 2) {
        return base * base;
    } else if constexpr (Exponent == 3) {
        return base * power<2>(base);
    } else if constexpr (Exponent == 4) {
        auto const x2 = power<2>(base);
        return x2 * x2;
    } else if constexpr (Exponent == 5) {
        return base * power<4>(base);
    } else if constexpr (Exponent == 6) {
        return power<2>(base) * power<4>(base);
    } else if constexpr (Exponent == 7) {
        auto const x2 = power<2>(base);
        auto const x4 = x2 * x2;
        return base * x2 * x4;
    } else if constexpr (Exponent == 8) {
        auto const x2 = power<2>(base);
        auto const x3 = x2 * base;
        auto const x6 = x3 * x3;
        return x6 * x2;
    } else {
        return base * power<Exponent - 1>(base);
    }
}

}  // namespace grit
