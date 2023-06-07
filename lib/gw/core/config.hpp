#pragma once

#if defined(__GNUC__) || defined(__clang__)
    #define TA_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER) && !defined(__clang__)
    #define TA_ALWAYS_INLINE __forceinline
#else
    #define TA_ALWAYS_INLINE
#endif
