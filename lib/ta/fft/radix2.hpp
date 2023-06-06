#pragma once

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/cmath.hpp>
#include <etl/complex.hpp>
#include <etl/cstdint.hpp>
#include <etl/numbers.hpp>
#include <etl/span.hpp>
#include <etl/utility.hpp>

#include <cmath>

#if defined(__GNUC__) || defined(__clang__)
    #define TA_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER) && !defined(__clang__)
    #define TA_ALWAYS_INLINE __forceinline
#else
    #define TA_ALWAYS_INLINE
#endif

namespace etl
{

template<typename T>
inline TA_ALWAYS_INLINE auto doNotOptimize(T& value) -> void
{
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

template<typename T>
inline auto polar(T const& rho, T const& theta) -> complex<T>
{
    return complex<T>(rho * std::cos(theta), rho * std::sin(theta));
}

}  // namespace etl

namespace ta::fft
{

template<typename T>
auto power(T base, T exponent) -> T
{
    T result = 1;
    for (T i = 0; i < exponent; i++) { result *= base; }
    return result;
}

template<auto Base>
auto power(decltype(Base) exponent) -> decltype(Base)
{
    return power(Base, exponent);
}

template<typename Float, unsigned Size>
auto make_twiddle_factors(bool inverse = false) -> etl::array<etl::complex<Float>, Size / 2>
{
    auto const sign = inverse ? Float(1) : Float(-1);
    auto table      = etl::array<etl::complex<Float>, Size / 2>{};
    for (unsigned i = 0; i < Size / 2; ++i)
    {
        auto const angle = sign * Float(2) * static_cast<Float>(etl::numbers::pi) * Float(i) / Float(Size);
        table[i]         = etl::polar(Float(1), angle);
    }
    return table;
}

template<typename Float, int Size>
struct twiddle_factor_lut
{
    twiddle_factor_lut() noexcept : _lut{make_twiddle_factors<Float, Size>()} {}

    auto operator[](int index) const -> etl::complex<Float> { return _lut[static_cast<etl::size_t>(index)]; }

private:
    etl::array<etl::complex<Float>, Size / 2> _lut;
};

template<typename Float, etl::size_t Extent>
auto radix2_inplace(etl::span<etl::complex<Float>, Extent> x, etl::span<etl::complex<Float> const, Extent / 2> w)
    -> void
{
    // bit-reverse ordering
    auto const len   = x.size();
    auto const order = static_cast<int>(std::lround(std::log2(len)));

    // // Rearrange the input in bit-reversed order
    // unsigned j = 0;
    // for (unsigned i = 0; i < len - 1U; ++i)
    // {
    //     if (i < j) { etl::swap(x[i], x[j]); }
    //     unsigned k = len / 2;
    //     while (k <= j)
    //     {
    //         j -= k;
    //         k /= 2;
    //     }
    //     j += k;
    // }

    // butterfly computation
    for (auto stage = 0; stage < order; ++stage)
    {

        auto const stage_length = power<2>(stage);
        auto const stride       = power<2>(stage + 1);
        auto const tw_stride    = power<2>(order - stage - 1);

        for (auto k = 0; etl::cmp_less(k, len); k += stride)
        {
            for (auto pair = 0; pair < stage_length; ++pair)
            {
                auto const tw = w[pair * tw_stride];

                auto const i1 = k + pair;
                auto const i2 = k + pair + stage_length;

                auto const temp = x[i1] + tw * x[i2];
                x[i2]           = x[i1] - tw * x[i2];
                x[i1]           = temp;
            }
        }
    }
}

template<typename Float, etl::size_t Size>
auto radix2_inplace(etl::span<etl::complex<Float>, Size> x, twiddle_factor_lut<Float, Size> const& w) -> void
{
    auto const len   = x.size();
    auto const order = static_cast<int>(std::lround(std::log2(len)));

    // butterfly computation
    for (auto stage = 0; stage < order; ++stage)
    {

        auto const stage_length = power<2>(stage);
        auto const stride       = power<2>(stage + 1);
        auto const tw_stride    = power<2>(order - stage - 1);

        for (auto k = 0; etl::cmp_less(k, len); k += stride)
        {
            for (auto pair = 0; pair < stage_length; ++pair)
            {
                auto const tw = w[pair * tw_stride];

                auto const i1 = k + pair;
                auto const i2 = k + pair + stage_length;

                auto const temp = x[i1] + tw * x[i2];
                x[i2]           = x[i1] - tw * x[i2];
                x[i1]           = temp;
            }
        }
    }
}

#define Q31_MAX ((::etl::int64_t)(0x7FFFFFFFL))
#define Q15_MAX ((::etl::int32_t)(0x7FFF))
#define Q7_MAX ((::etl::int32_t)(0x7F))
#define Q31_MIN (-(::etl::int64_t)(0x80000000L))
#define Q15_MIN (-(::etl::int32_t)(0x8000))
#define Q7_MIN (-(::etl::int32_t)(0x80))

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

TA_ALWAYS_INLINE inline auto smlad(etl::uint32_t op1, etl::uint32_t op2, etl::uint32_t op3) -> etl::uint32_t
{
    etl::uint32_t result;
    __asm volatile("smlad %0, %1, %2, %3" : "=r"(result) : "r"(op1), "r"(op2), "r"(op3));
    return result;
}

TA_ALWAYS_INLINE inline auto smlsd(etl::uint32_t op1, etl::uint32_t op2, etl::uint32_t op3) -> etl::uint32_t
{
    etl::uint32_t result;
    __asm volatile("smlsd %0, %1, %2, %3" : "=r"(result) : "r"(op1), "r"(op2), "r"(op3));
    return result;
}

template<typename StorageType>
auto saturate(etl::int32_t x) -> StorageType
{
    // #if __arm__
    //     // etl::int32_t depth;
    //     // if constexpr (std::is_same_v<StorageType, etl::int8_t>) { depth = 8; }
    //     // else if constexpr (std::is_same_v<StorageType, etl::int16_t>) { depth = 16; }
    //     // else { depth = 32; }

    //     etl::int32_t result;
    //     // __asm volatile("ssat %[result], %[a], %[b]" : [result] "=r"(result) : [a] "r"(x), [b] "r"(depth));
    //     __asm volatile("ssat %0, %1, %2" : "=r"(result) : "r"(x), "r"(16));
    //     return result;

    // #else
    if constexpr (std::is_same_v<StorageType, etl::int8_t>)
    {
        return static_cast<etl::int8_t>(etl::clamp(x, Q7_MIN, Q7_MAX));
    }
    // else if constexpr (std::is_same_v<StorageType, etl::int16_t>)
    // {
    // }
    return static_cast<etl::int16_t>(etl::clamp(x, Q15_MIN, Q15_MAX));

    // return static_cast<etl::int32_t>(etl::clamp(x, Q31_MIN, Q31_MAX));
    // #endif
}

constexpr auto pkhbt(etl::int16_t arg1, etl::int16_t arg2, etl::uint32_t shift) -> etl::uint32_t
{
    auto const bottom = static_cast<etl::uint32_t>(arg1) & etl::uint32_t(0x0000FFFFUL);
    auto const top    = (static_cast<etl::uint32_t>(arg2) << shift) & etl::uint32_t(0xFFFF0000UL);
    return bottom | top;
}

struct complex_q15_t
{
    constexpr complex_q15_t(etl::int16_t re = etl::int16_t(0), etl::int16_t im = etl::int16_t(0)) noexcept
        : _data{pkhbt(re, im, 16)}
    {
    }

    // constexpr auto operator=(etl::int16_t const& val) -> complex_q15_t& {
    //     real(val);
    //     return *this;
    // }

    [[nodiscard]] constexpr auto real() const -> etl::int16_t
    {
        return static_cast<etl::int16_t>((_data & etl::uint32_t(0xFFFF0000UL)) >> 16);
    }
    [[nodiscard]] constexpr auto imag() const -> etl::int16_t
    {
        return static_cast<etl::int16_t>(_data & etl::uint32_t(0x0000FFFFUL));
    }

    // constexpr auto real(etl::int16_t val) -> void { _data[0] = val; }
    // constexpr auto imag(etl::int16_t val) -> void { _data[1] = val; }

    TA_ALWAYS_INLINE friend auto operator+(complex_q15_t lhs, complex_q15_t rhs) noexcept -> complex_q15_t
    {
        auto r  = complex_q15_t{};
        r._data = qadd16(lhs._data, rhs._data);
        return r;
    }

    TA_ALWAYS_INLINE friend auto operator-(complex_q15_t lhs, complex_q15_t rhs) noexcept -> complex_q15_t
    {
        auto r  = complex_q15_t{};
        r._data = qsub16(lhs._data, rhs._data);
        return r;
    }

    TA_ALWAYS_INLINE friend auto operator*(complex_q15_t lhs, complex_q15_t rhs) noexcept -> complex_q15_t
    {
        // auto im = static_cast<etl::int16_t>(smlad(lhs._data, rhs._data, 0));
        // auto re = static_cast<etl::int16_t>(smlsd(lhs._data, rhs._data, 0));
        // return complex_q15_t{saturate<etl::int16_t>(re), saturate<etl::int16_t>(im)};
        etl::int16_t re = lhs.real() * rhs.real() - lhs.imag() * rhs.imag();
        etl::int16_t im = lhs.real() * rhs.imag() + lhs.imag() * rhs.real();
        return complex_q15_t{re, im};
    }

private:
    etl::uint32_t _data;
};

template<etl::size_t Extent>
auto radix2_inplace(etl::span<complex_q15_t, Extent> x, etl::span<complex_q15_t const, Extent / 2> w) -> void
{
    auto const len   = x.size();
    auto const order = static_cast<int>(std::lround(std::log2(len)));

    // butterfly computation
    for (auto stage = 0; stage < order; ++stage)
    {
        auto const stage_length = power<2>(stage);
        auto const stride       = power<2>(stage + 1);
        auto const tw_stride    = power<2>(order - stage - 1);

        for (auto k = 0; etl::cmp_less(k, len); k += stride)
        {
            for (auto pair = 0; pair < stage_length; ++pair)
            {
                auto const tw = w[pair * tw_stride];

                auto const i1 = k + pair;
                auto const i2 = k + pair + stage_length;

                auto const temp = x[i1] + tw * x[i2];
                x[i2]           = x[i1] - tw * x[i2];
                x[i1]           = temp;
            }
        }
    }
}

}  // namespace ta::fft
