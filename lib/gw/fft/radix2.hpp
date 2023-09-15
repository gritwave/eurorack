#pragma once

#include <gw/math/complex.hpp>
#include <gw/math/ipow.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/cmath.hpp>
#include <etl/complex.hpp>
#include <etl/cstdint.hpp>
#include <etl/numbers.hpp>
#include <etl/span.hpp>
#include <etl/utility.hpp>

#include <cmath>

namespace gw::fft
{

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
    auto const order = static_cast<int>(std::lround(etl::log2(len)));

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

        auto const stage_length = ipow<2>(stage);
        auto const stride       = ipow<2>(stage + 1);
        auto const tw_stride    = ipow<2>(order - stage - 1);

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

template<typename Float, etl::size_t Extent>
auto radix2_inplace_alt(etl::span<etl::complex<Float>, Extent> x, etl::span<etl::complex<Float> const, Extent / 2> w)
{
    auto const len = x.size();

    auto stage_size = 2U;
    while (stage_size <= len)
    {
        auto const halfStage = stage_size / 2;
        auto const k_step    = len / stage_size;

        for (auto i{0U}; i < len; i += stage_size)
        {
            for (auto k{i}; k < i + halfStage; ++k)
            {
                auto const index = k - i;
                auto const tw    = w[index * k_step];

                auto const idx1 = k;
                auto const idx2 = k + halfStage;

                auto const even = x[idx1];
                auto const odd  = x[idx2];

                auto const tmp = odd * tw;
                x[idx1]        = even + tmp;
                x[idx2]        = even - tmp;
            }
        }

        stage_size *= 2;
    }
}

template<typename Float, etl::size_t Size>
auto radix2_inplace(etl::span<etl::complex<Float>, Size> x, twiddle_factor_lut<Float, Size> const& w) -> void
{
    auto const len   = x.size();
    auto const order = static_cast<int>(std::lround(etl::log2(len)));

    // butterfly computation
    for (auto stage = 0; stage < order; ++stage)
    {

        auto const stage_length = ipow<2>(stage);
        auto const stride       = ipow<2>(stage + 1);
        auto const tw_stride    = ipow<2>(order - stage - 1);

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

template<etl::size_t Extent>
auto radix2_inplace(etl::span<complex_q15_t, Extent> x, etl::span<complex_q15_t const, Extent / 2> w) -> void
{
    // auto const len   = x.size();
    // auto const order = static_cast<int>(std::lround(etl::log2(len)));

    // // butterfly computation
    // for (auto stage = 0; stage < order; ++stage)
    // {
    //     auto const stage_length = ipow<2>(stage);
    //     auto const stride       = ipow<2>(stage + 1);
    //     auto const tw_stride    = ipow<2>(order - stage - 1);

    //     for (auto k = 0; etl::cmp_less(k, len); k += stride)
    //     {
    //         for (auto pair = 0; pair < stage_length; ++pair)
    //         {
    //             auto const tw = w[pair * tw_stride];

    //             auto const i1 = k + pair;
    //             auto const i2 = k + pair + stage_length;

    //             auto const temp = x[i1] + tw * x[i2];
    //             x[i2]           = x[i1] - tw * x[i2];
    //             x[i1]           = temp;
    //         }
    //     }
    // }

    auto const len = x.size();

    auto stage_size = 2U;
    while (stage_size <= len)
    {
        auto const halfStage = stage_size / 2;
        auto const k_step    = len / stage_size;

        for (auto i{0U}; i < len; i += stage_size)
        {
            for (auto k{i}; k < i + halfStage; ++k)
            {
                auto const index = k - i;
                auto const tw    = w[index * k_step];

                auto const idx1 = k;
                auto const idx2 = k + halfStage;

                auto const even = x[idx1];
                auto const odd  = x[idx2];

                auto const tmp = odd * tw;
                x[idx1]        = even + tmp;
                x[idx2]        = even - tmp;
            }
        }

        stage_size *= 2;
    }
}

}  // namespace gw::fft
