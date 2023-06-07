#pragma once

#include <gw/core/arm.hpp>

#include <etl/cstdint.hpp>

namespace gw
{

struct complex_q15_t
{
    constexpr complex_q15_t(etl::int16_t re = etl::int16_t(0), etl::int16_t im = etl::int16_t(0)) noexcept
        : _data{arm::pkhbt(re, im, 16)}
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
        r._data = arm::qadd16(lhs._data, rhs._data);
        return r;
    }

    TA_ALWAYS_INLINE friend auto operator-(complex_q15_t lhs, complex_q15_t rhs) noexcept -> complex_q15_t
    {
        auto r  = complex_q15_t{};
        r._data = arm::qsub16(lhs._data, rhs._data);
        return r;
    }

    TA_ALWAYS_INLINE friend auto operator*(complex_q15_t lhs, complex_q15_t rhs) noexcept -> complex_q15_t
    {
        auto const lr = static_cast<etl::int32_t>(lhs.real());
        auto const li = static_cast<etl::int32_t>(lhs.imag());
        auto const rr = static_cast<etl::int32_t>(rhs.real());
        auto const ri = static_cast<etl::int32_t>(rhs.imag());

        auto const re = static_cast<etl::int16_t>(((lr * rr) >> 17) - ((li * ri) >> 17));
        auto const im = static_cast<etl::int16_t>(((lr * ri) >> 17) + ((li * rr) >> 17));
        return complex_q15_t{re, im};
    }

private:
    etl::uint32_t _data;
};

}  // namespace gw
