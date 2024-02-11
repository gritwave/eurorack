#pragma once

#include <grit/math/linear_interpolation.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-math
template<etl::floating_point Float, etl::size_t Size>
struct StaticLookupTable
{
    using value_type = Float;
    using size_type  = etl::size_t;

    constexpr StaticLookupTable() = default;

    template<etl::regular_invocable<etl::size_t> FunctionToApproximate>
        requires(etl::same_as<etl::invoke_result_t<FunctionToApproximate, etl::size_t>, Float>)
    explicit constexpr StaticLookupTable(FunctionToApproximate func)
    {
        initialize(func);
    }

    template<etl::regular_invocable<etl::size_t> FunctionToApproximate>
        requires(etl::same_as<etl::invoke_result_t<FunctionToApproximate, etl::size_t>, Float>)
    constexpr auto initialize(FunctionToApproximate func) -> void
    {
        for (auto i = etl::size_t{0}; i < Size; ++i) {
            auto value = func(i);
            _buffer[i] = value;
        }
        _buffer[Size] = _buffer[Size - 1];
    }

    [[nodiscard]] constexpr auto atUnchecked(Float index) const -> Float
    {
        auto const i  = static_cast<etl::size_t>(index);
        auto const f  = index - Float(i);
        auto const x0 = _buffer[i];
        auto const x1 = _buffer[i + 1];
        return linearInterpolation(x0, x1, f);
    }

    [[nodiscard]] constexpr auto at(Float index) const -> Float
    {
        return atUnchecked(etl::clamp(index, Float(0), Float(Size - 1)));
    }

    [[nodiscard]] constexpr auto operator[](Float index) const -> Float { return atUnchecked(index); }

    [[nodiscard]] static constexpr auto size() -> etl::size_t { return Size; }

private:
    etl::array<Float, Size + 1> _buffer;
};

}  // namespace grit
