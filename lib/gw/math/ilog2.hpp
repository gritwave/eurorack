#pragma once

#include <etl/concepts.hpp>

namespace gw {

template<etl::integral Int>
[[nodiscard]] constexpr auto ilog2(Int x) noexcept -> Int
{
    auto result = Int{0};
    for (; x > Int(1); x >>= Int(1)) {
        ++result;
    }
    return result;
}

}  // namespace gw
