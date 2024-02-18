#pragma once

#include <etl/concepts.hpp>
#include <etl/type_traits.hpp>

namespace grit {

/// \ingroup grit-math
template<etl::integral Int>
[[nodiscard]] constexpr auto ipow(Int base, Int exponent) -> Int
{
    Int result = 1;
    for (Int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

/// \ingroup grit-math
template<etl::integral auto Base>
    requires(Base > 0)
[[nodiscard]] constexpr auto ipow(decltype(Base) exponent) -> decltype(Base)
{
    using Int  = decltype(Base);
    using UInt = etl::make_unsigned_t<Int>;

    if constexpr (Base == Int{2}) {
        return static_cast<Int>(UInt(1) << static_cast<UInt>(exponent));
    } else {
        return ipow(Base, exponent);
    }
}

}  // namespace grit
