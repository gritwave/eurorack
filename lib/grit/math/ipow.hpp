#pragma once

#include <etl/concepts.hpp>

namespace grit {

template<etl::integral Int>
[[nodiscard]] constexpr auto ipow(Int base, Int exponent) noexcept -> Int
{
    Int result = 1;
    for (Int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

template<auto Base>
    requires(etl::integral<decltype(Base)> and Base > 0)
[[nodiscard]] constexpr auto ipow(decltype(Base) exponent) noexcept -> decltype(Base)
{
    using Int = decltype(Base);
    if constexpr (Base == Int{2}) {
        return static_cast<Int>(1U << static_cast<unsigned>(exponent));
    } else {
        return ipow(Base, exponent);
    }
}

}  // namespace grit
