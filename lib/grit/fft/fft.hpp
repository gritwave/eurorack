#pragma once

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

namespace detail {

template<typename Float, unsigned Size>
auto makeTwiddles(Direction dir = Direction::Forward) -> etl::array<etl::complex<Float>, Size / 2>
{
    auto const sign = dir == Direction::Forward ? Float(-1) : Float(1);
    auto table      = etl::array<etl::complex<Float>, Size / 2>{};
    for (unsigned i = 0; i < Size / 2; ++i) {
        auto const angle = sign * Float(2) * static_cast<Float>(etl::numbers::pi) * Float(i) / Float(Size);
        table[i]         = etl::polar(Float(1), angle);
    }
    return table;
}

template<typename Complex, int Order, int Stage>
struct ComplexDit2Stage
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

        ComplexDit2Stage<Complex, Order, 1>{}(x, w);
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

        ComplexDit2Stage<Complex, Order, Stage + 1>{}(x, w);
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

/// \ingroup grit-fft
template<typename Complex, etl::size_t Size>
struct ComplexPlan
{
    using ValueType = Complex;
    using size_type = etl::size_t;

    explicit ComplexPlan(Direction defaultDirection = Direction::Forward)
        : _defaultDirection{defaultDirection}
        , _w{detail::makeTwiddles<typename Complex::value_type, size()>(defaultDirection)}
    {}

    [[nodiscard]] static constexpr auto size() -> etl::size_t { return Size; }

    [[nodiscard]] static constexpr auto order() -> etl::size_t { return ilog2(Size); }

    template<etl::linalg::inout_vector InOutVec>
        requires etl::same_as<typename InOutVec::value_type, Complex>
    auto operator()(InOutVec x, Direction dir) -> void
    {
        _reorder(x);

        auto const w = etl::mdspan<Complex, etl::extents<etl::size_t, size()>>{_w.data()};

        if (dir == _defaultDirection) {
            detail::ComplexDit2Stage<Complex, order(), 0>{}(x, w);
        } else {
            detail::ComplexDit2Stage<Complex, order(), 0>{}(x, etl::linalg::conjugated(w));
        }
    }

private:
    Direction _defaultDirection;
    StaticBitrevorderPlan<size()> _reorder{};
    etl::array<Complex, size() / 2> _w;
};

/// \ingroup grit-fft
template<typename Complex, etl::size_t Size>
struct ComplexPlanV2
{
    using ValueType = Complex;
    using size_type = etl::size_t;

    explicit ComplexPlanV2(Direction defaultDirection = Direction::Forward)
        : _defaultDirection{defaultDirection}
        , _w{detail::makeTwiddles<typename Complex::value_type, size()>(defaultDirection)}
    {}

    [[nodiscard]] static constexpr auto size() -> etl::size_t { return Size; }

    [[nodiscard]] static constexpr auto order() -> etl::size_t { return ilog2(Size); }

    template<etl::linalg::inout_vector InOutVec>
        requires etl::same_as<typename InOutVec::value_type, Complex>
    auto operator()(InOutVec x, Direction dir) -> void
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
    Direction _defaultDirection;
    StaticBitrevorderPlan<size()> _reorder{};
    etl::array<Complex, size() / 2> _w;
};

}  // namespace grit::fft
