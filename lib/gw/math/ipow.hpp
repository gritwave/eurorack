#pragma once

#include <etl/concepts.hpp>

namespace gw
{

template<etl::integral Int>
[[nodiscard]] constexpr auto ipow(Int base, Int exponent) noexcept -> Int
{
    Int result = 1;
    for (Int i = 0; i < exponent; i++) { result *= base; }
    return result;
}

template<auto Base>
[[nodiscard]] constexpr auto ipow(decltype(Base) exponent) noexcept -> decltype(Base)
{
    return ipow(Base, exponent);
}

}  // namespace gw
