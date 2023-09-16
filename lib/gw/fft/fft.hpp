#pragma once

#include <gw/math/complex.hpp>
#include <gw/math/ipow.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/cmath.hpp>
#include <etl/complex.hpp>
#include <etl/cstdint.hpp>
#include <etl/mdspan.hpp>
#include <etl/numbers.hpp>
#include <etl/span.hpp>
#include <etl/utility.hpp>

#include <cmath>

namespace gw::fft {

template<typename Float, unsigned Size>
auto make_twiddle_factors(bool inverse = false) -> etl::array<etl::complex<Float>, Size / 2>
{
    auto const sign = inverse ? Float(1) : Float(-1);
    auto table      = etl::array<etl::complex<Float>, Size / 2>{};
    for (unsigned i = 0; i < Size / 2; ++i) {
        auto const angle = sign * Float(2) * static_cast<Float>(etl::numbers::pi) * Float(i) / Float(Size);
        table[i]         = etl::polar(Float(1), angle);
    }
    return table;
}

struct c2c_dit2_v1
{
    c2c_dit2_v1() = default;

    template<typename Vec>
        requires(Vec::rank() == 1)
    auto operator()(Vec x, auto const& w) -> void
    {
        auto const len   = x.extent(0);
        auto const order = static_cast<int>(std::lround(etl::log2(len)));

        for (auto stage = 0; stage < order; ++stage) {

            auto const stage_length = ipow<2>(stage);
            auto const stride       = ipow<2>(stage + 1);
            auto const tw_stride    = ipow<2>(order - stage - 1);

            for (auto k = 0; etl::cmp_less(k, len); k += stride) {
                for (auto pair = 0; pair < stage_length; ++pair) {
                    auto const tw = w(pair * tw_stride);

                    auto const i1 = k + pair;
                    auto const i2 = k + pair + stage_length;

                    auto const temp = x(i1) + tw * x(i2);
                    x(i2)           = x(i1) - tw * x(i2);
                    x(i1)           = temp;
                }
            }
        }
    }
};

struct c2c_dit2_v2
{
    c2c_dit2_v2() = default;

    template<typename Vec>
        requires(Vec::rank() == 1)
    auto operator()(Vec x, auto const& w) -> void
    {
        auto const len = x.extent(0);

        auto stage_size = 2U;
        while (stage_size <= len) {
            auto const halfStage = stage_size / 2;
            auto const k_step    = len / stage_size;

            for (auto i{0U}; i < len; i += stage_size) {
                for (auto k{i}; k < i + halfStage; ++k) {
                    auto const index = k - i;
                    auto const tw    = w(index * k_step);

                    auto const idx1 = k;
                    auto const idx2 = k + halfStage;

                    auto const even = x(idx1);
                    auto const odd  = x(idx2);

                    auto const tmp = odd * tw;
                    x(idx1)        = even + tmp;
                    x(idx2)        = even - tmp;
                }
            }

            stage_size *= 2;
        }
    }
};

struct c2c_dit2_v3
{
    c2c_dit2_v3() = default;

    template<typename Vec>
        requires(Vec::rank() == 1)
    auto operator()(Vec x, auto const& w) -> void
    {
        auto const len   = x.extent(0);
        auto const order = static_cast<int>(std::lround(etl::log2(len)));

        {
            // stage 0
            static constexpr auto const stage_length = 1;  // ipow<2>(0)
            static constexpr auto const stride       = 2;  // ipow<2>(0 + 1)

            for (auto k{0}; k < static_cast<int>(len); k += stride) {
                auto const i1 = k;
                auto const i2 = k + stage_length;

                auto const temp = x(i1) + x(i2);
                x(i2)           = x(i1) - x(i2);
                x(i1)           = temp;
            }
        }

        for (auto stage = 1; stage < order; ++stage) {

            auto const stage_length = ipow<2>(stage);
            auto const stride       = ipow<2>(stage + 1);
            auto const tw_stride    = ipow<2>(order - stage - 1);

            for (auto k = 0; etl::cmp_less(k, len); k += stride) {
                for (auto pair = 0; pair < stage_length; ++pair) {
                    auto const tw = w(pair * tw_stride);

                    auto const i1 = k + pair;
                    auto const i2 = k + pair + stage_length;

                    auto const temp = x(i1) + tw * x(i2);
                    x(i2)           = x(i1) - tw * x(i2);
                    x(i1)           = temp;
                }
            }
        }
    }
};

}  // namespace gw::fft
