#pragma once

#include <ta/core/config.hpp>

#include <etl/algorithm.hpp>
#include <etl/cstdint.hpp>
#include <etl/type_traits.hpp>

#define TA_Q31_MAX ((::etl::int64_t)(0x7FFFFFFFL))
#define TA_Q15_MAX ((::etl::int32_t)(0x7FFF))
#define TA_Q7_MAX ((::etl::int32_t)(0x7F))
#define TA_Q31_MIN (-(::etl::int64_t)(0x80000000L))
#define TA_Q15_MIN (-(::etl::int32_t)(0x8000))
#define TA_Q7_MIN (-(::etl::int32_t)(0x80))

namespace ta::arm
{

TA_ALWAYS_INLINE inline auto qadd16(etl::uint32_t op1, etl::uint32_t op2) -> etl::uint32_t
{
    auto result = etl::uint32_t{};
    __asm volatile("qadd16 %0, %1, %2" : "=r"(result) : "r"(op1), "r"(op2));
    return result;
}

TA_ALWAYS_INLINE inline auto qsub16(etl::uint32_t op1, etl::uint32_t op2) -> etl::uint32_t
{
    auto result = etl::uint32_t{};
    __asm volatile("qsub16 %0, %1, %2" : "=r"(result) : "r"(op1), "r"(op2));
    return result;
}

TA_ALWAYS_INLINE inline auto smuad(etl::uint32_t op1, etl::uint32_t op2) -> etl::uint32_t
{
    auto result = etl::uint32_t{};
    __asm volatile("smuad %0, %1, %2" : "=r"(result) : "r"(op1), "r"(op2));
    return result;
}

TA_ALWAYS_INLINE inline auto smuadx(etl::uint32_t op1, etl::uint32_t op2) -> etl::uint32_t
{
    auto result = etl::uint32_t{};
    __asm volatile("smuadx %0, %1, %2" : "=r"(result) : "r"(op1), "r"(op2));
    return result;
}

TA_ALWAYS_INLINE inline auto smusd(etl::uint32_t op1, etl::uint32_t op2) -> etl::uint32_t
{
    auto result = etl::uint32_t{};
    __asm volatile("smusd %0, %1, %2" : "=r"(result) : "r"(op1), "r"(op2));
    return result;
}

TA_ALWAYS_INLINE inline auto smusdx(etl::uint32_t op1, etl::uint32_t op2) -> etl::uint32_t
{
    auto result = etl::uint32_t{};
    __asm volatile("smusdx %0, %1, %2" : "=r"(result) : "r"(op1), "r"(op2));
    return result;
}

TA_ALWAYS_INLINE inline auto ssat16(etl::int32_t x) -> etl::int16_t
{
#if __arm__
    auto result = etl::int32_t{};
    __asm volatile("ssat %0, %1, %2" : "=r"(result) : "I"(16), "r"(x));
    return result;

#else
    return static_cast<etl::int16_t>(etl::clamp(x, TA_Q15_MIN, TA_Q15_MAX));
#endif
}

constexpr auto pkhbt(etl::int16_t arg1, etl::int16_t arg2, etl::uint32_t shift) -> etl::uint32_t
{
    auto const bottom = static_cast<etl::uint32_t>(arg1) & etl::uint32_t(0x0000FFFFUL);
    auto const top    = (static_cast<etl::uint32_t>(arg2) << shift) & etl::uint32_t(0xFFFF0000UL);
    return bottom | top;
}

}  // namespace ta::arm
