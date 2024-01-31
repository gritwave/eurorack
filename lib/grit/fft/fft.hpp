#pragma once

#include <grit/core/mdspan.hpp>
#include <grit/fft/bitrevorder.hpp>
#include <grit/fft/direction.hpp>
#include <grit/math/ilog2.hpp>
#include <grit/math/ipow.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/cmath.hpp>
#include <etl/complex.hpp>
#include <etl/cstdint.hpp>
#include <etl/linalg.hpp>
#include <etl/mdspan.hpp>
#include <etl/numbers.hpp>
#include <etl/span.hpp>
#include <etl/utility.hpp>

namespace grit::fft {

template<typename Float, unsigned Size>
auto makeTwiddlesR2(direction dir = direction::forward) -> etl::array<etl::complex<Float>, Size / 2>
{
    auto const sign = dir == direction::forward ? Float(-1) : Float(1);
    auto table      = etl::array<etl::complex<Float>, Size / 2>{};
    for (unsigned i = 0; i < Size / 2; ++i) {
        auto const angle = sign * Float(2) * static_cast<Float>(etl::numbers::pi) * Float(i) / Float(Size);
        table[i]         = etl::polar(Float(1), angle);
    }
    return table;
}

struct C2cDit2V1
{
    C2cDit2V1() = default;

    template<typename Vec>
        requires(Vec::rank() == 1)
    auto operator()(Vec x, auto const& w) -> void
    {
        auto const len   = x.extent(0);
        auto const order = ilog2(len);

        for (auto stage = 0; stage < order; ++stage) {

            auto const stageLength = ipow<2>(stage);
            auto const stride      = ipow<2>(stage + 1);
            auto const twStride    = ipow<2>(order - stage - 1);

            for (auto k = 0; etl::cmp_less(k, len); k += stride) {
                for (auto pair = 0; pair < stageLength; ++pair) {
                    auto const tw = w(pair * twStride);

                    auto const i1 = k + pair;
                    auto const i2 = k + pair + stageLength;

                    auto const temp = x(i1) + tw * x(i2);
                    x(i2)           = x(i1) - tw * x(i2);
                    x(i1)           = temp;
                }
            }
        }
    }
};

struct C2cDit2V2
{
    C2cDit2V2() = default;

    template<typename Vec>
        requires(Vec::rank() == 1)
    auto operator()(Vec x, auto const& w) -> void
    {
        auto const len = x.extent(0);

        auto stageSize = 2U;
        while (stageSize <= len) {
            auto const halfStage = stageSize / 2;
            auto const kStep     = len / stageSize;

            for (auto i{0U}; i < len; i += stageSize) {
                for (auto k{i}; k < i + halfStage; ++k) {
                    auto const index = k - i;
                    auto const tw    = w(index * kStep);

                    auto const idx1 = k;
                    auto const idx2 = k + halfStage;

                    auto const even = x(idx1);
                    auto const odd  = x(idx2);

                    auto const tmp = odd * tw;
                    x(idx1)        = even + tmp;
                    x(idx2)        = even - tmp;
                }
            }

            stageSize *= 2;
        }
    }
};

struct C2cDit2V3
{
    C2cDit2V3() = default;

    template<typename Vec>
        requires(Vec::rank() == 1)
    auto operator()(Vec x, auto const& w) -> void
    {
        auto const len   = static_cast<int>(x.extent(0));
        auto const order = ilog2(len);

        {
            // stage 0
            static constexpr auto const stageLength = 1;  // ipow<2>(0)
            static constexpr auto const stride      = 2;  // ipow<2>(0 + 1)

            for (auto k{0}; k < static_cast<int>(len); k += stride) {
                auto const i1 = k;
                auto const i2 = k + stageLength;

                auto const temp = x(i1) + x(i2);
                x(i2)           = x(i1) - x(i2);
                x(i1)           = temp;
            }
        }

        for (auto stage = 1; stage < order; ++stage) {

            auto const stageLength = ipow<2>(stage);
            auto const stride      = ipow<2>(stage + 1);
            auto const twStride    = ipow<2>(order - stage - 1);

            for (auto k = 0; etl::cmp_less(k, len); k += stride) {
                for (auto pair = 0; pair < stageLength; ++pair) {
                    auto const tw = w(pair * twStride);

                    auto const i1 = k + pair;
                    auto const i2 = k + pair + stageLength;

                    auto const temp = x(i1) + tw * x(i2);
                    x(i2)           = x(i1) - tw * x(i2);
                    x(i1)           = temp;
                }
            }
        }
    }
};

namespace detail {
template<typename Complex, int Order, int Stage>
struct StaticC2cDit2Stage
{
    auto operator()(etl::linalg::inout_vector auto x, etl::linalg::in_vector auto w) -> void
        requires(Stage == 0)
    {
        static constexpr auto const size        = 1 << Order;
        static constexpr auto const stageLength = 1;  // ipow<2>(0)
        static constexpr auto const stride      = 2;  // ipow<2>(0 + 1)

        for (auto k{0}; k < static_cast<int>(size); k += stride) {
            auto const i1 = k;
            auto const i2 = k + stageLength;

            auto const temp = x(i1) + x(i2);
            x(i2)           = x(i1) - x(i2);
            x(i1)           = temp;
        }

        StaticC2cDit2Stage<Complex, Order, 1>{}(x, w);
    }

    auto operator()(etl::linalg::inout_vector auto x, etl::linalg::in_vector auto w) -> void
        requires(Stage != 0 and Stage < Order)
    {
        static constexpr auto const size        = 1 << Order;
        static constexpr auto const stageLength = ipow<2>(Stage);
        static constexpr auto const stride      = ipow<2>(Stage + 1);
        static constexpr auto const twStride    = ipow<2>(Order - Stage - 1);

        for (auto k{0}; k < size; k += stride) {
            for (auto pair{0}; pair < stageLength; ++pair) {
                auto const tw = w(pair * twStride);

                auto const i1 = k + pair;
                auto const i2 = k + pair + stageLength;

                auto const temp = x(i1) + tw * x(i2);
                x(i2)           = x(i1) - tw * x(i2);
                x(i1)           = temp;
            }
        }

        StaticC2cDit2Stage<Complex, Order, Stage + 1>{}(x, w);
    }

    auto operator()(etl::linalg::inout_vector auto /*x*/, etl::linalg::in_vector auto /*w*/) -> void
        requires(Stage == Order)
    {}
};

template<int Stage, etl::linalg::inout_vector InOutVec, etl::linalg::in_vector InVec>
    requires(Stage == 0)
[[gnu::noinline]] auto staticDit2StageV2(InOutVec x, InVec /*w*/, int order) -> void
{
    static constexpr auto const stageLength = 1;  // ipow<2>(0)
    static constexpr auto const stride      = 2;  // ipow<2>(0 + 1)

    auto const size = 1 << order;

    for (auto k{0}; k < static_cast<int>(size); k += stride) {
        auto const i1 = k;
        auto const i2 = k + stageLength;

        auto const temp = x(i1) + x(i2);
        x(i2)           = x(i1) - x(i2);
        x(i1)           = temp;
    }
}

template<int Stage, etl::linalg::inout_vector InOutVec, etl::linalg::in_vector InVec>
    requires(Stage != 0)
[[gnu::noinline]] auto staticDit2StageV2(InOutVec x, InVec w, int order) -> void
{
    static constexpr auto const stageLength = ipow<2>(Stage);
    static constexpr auto const stride      = ipow<2>(Stage + 1);

    auto const size     = 1 << order;
    auto const twStride = ipow<2>(order - Stage - 1);

    for (auto k{0}; k < size; k += stride) {
        for (auto pair{0}; pair < stageLength; ++pair) {
            auto const tw = w(pair * twStride);

            auto const i1 = k + pair;
            auto const i2 = k + pair + stageLength;

            auto const temp = x(i1) + tw * x(i2);
            x(i2)           = x(i1) - tw * x(i2);
            x(i1)           = temp;
        }
    }
}

}  // namespace detail

template<typename Complex, etl::size_t Size>
struct StaticFftPlan
{
    using value_type = Complex;
    using size_type  = etl::size_t;

    explicit StaticFftPlan(direction defaultDirection = direction::forward) noexcept
        : _defaultDirection{defaultDirection}
        , _w{makeTwiddlesR2<typename Complex::value_type, size()>(defaultDirection)}
    {}

    [[nodiscard]] static constexpr auto size() noexcept -> etl::size_t { return Size; }

    [[nodiscard]] static constexpr auto order() noexcept -> etl::size_t { return ilog2(Size); }

    template<etl::linalg::inout_vector InOutVec>
        requires etl::same_as<typename InOutVec::value_type, Complex>
    auto operator()(InOutVec x, direction dir) noexcept -> void
    {
        _reorder(x);

        auto const w = etl::mdspan<Complex, etl::extents<etl::size_t, size()>>{_w.data()};

        if (dir == _defaultDirection) {
            detail::StaticC2cDit2Stage<Complex, order(), 0>{}(x, w);
        } else {
            detail::StaticC2cDit2Stage<Complex, order(), 0>{}(x, etl::linalg::conjugated(w));
        }
    }

private:
    direction _defaultDirection;
    StaticBitrevorderPlan<size()> _reorder{};
    etl::array<Complex, size() / 2> _w;
};

template<typename Complex, etl::size_t Size>
struct StaticFftPlanV2
{
    using value_type = Complex;
    using size_type  = etl::size_t;

    explicit StaticFftPlanV2(direction defaultDirection = direction::forward) noexcept
        : _defaultDirection{defaultDirection}
        , _w{makeTwiddlesR2<typename Complex::value_type, size()>(defaultDirection)}
    {}

    [[nodiscard]] static constexpr auto size() noexcept -> etl::size_t { return Size; }

    [[nodiscard]] static constexpr auto order() noexcept -> etl::size_t { return ilog2(Size); }

    template<etl::linalg::inout_vector InOutVec>
        requires etl::same_as<typename InOutVec::value_type, Complex>
    auto operator()(InOutVec x, direction dir) noexcept -> void
    {
        auto runStages = [x]<etl::size_t... Stage>(etl::index_sequence<Stage...>, etl::linalg::in_vector auto w) {
            (detail::staticDit2StageV2<Stage>(x, w, order()), ...);
        };

        _reorder(x);

        auto const w = etl::mdspan<Complex, etl::extents<etl::size_t, size()>>{_w.data()};

        if (dir == _defaultDirection) {
            runStages(etl::make_index_sequence<order()>(), w);
        } else {
            runStages(etl::make_index_sequence<order()>(), etl::linalg::conjugated(w));
        }
    }

private:
    direction _defaultDirection;
    StaticBitrevorderPlan<size()> _reorder{};
    etl::array<Complex, size() / 2> _w;
};

}  // namespace grit::fft
