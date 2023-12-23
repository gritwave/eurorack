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
[[nodiscard]] constexpr auto ipow(decltype(Base) exponent) noexcept -> decltype(Base)
{
    using Int = decltype(Base);
    if constexpr (Base == Int{2}) {
        return static_cast<Int>(Int{1} << exponent);
    } else {
        return ipow(Base, exponent);
    }
}

}  // namespace grit
