#pragma once

#include <ta/core/arm.hpp>

#include <etl/cstdint.hpp>

namespace ta
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
        // auto im = static_cast<etl::int16_t>(arm::smlad(lhs._data, rhs._data, 0));
        // auto re = static_cast<etl::int16_t>(arm::smlsd(lhs._data, rhs._data, 0));
        // return complex_q15_t{arm::saturate<etl::int16_t>(re), arm::saturate<etl::int16_t>(im)};
        etl::int16_t re = lhs.real() * rhs.real() - lhs.imag() * rhs.imag();
        etl::int16_t im = lhs.real() * rhs.imag() + lhs.imag() * rhs.real();
        return complex_q15_t{re, im};
    }

private:
    etl::uint32_t _data;
};

}  // namespace ta
