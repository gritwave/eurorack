#pragma once

#include <etl/concepts.hpp>

namespace grit {

template<typename Type>
[[nodiscard]] constexpr auto sign(Type x) -> Type
{
    return static_cast<Type>((Type(0) < x) - (x < Type(0)));
}

}  // namespace grit
