#pragma once

#include <grit/core/config.hpp>

namespace grit {

template<typename T>
inline TA_ALWAYS_INLINE auto do_not_optimize(T& value) -> void
{
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

}  // namespace grit
