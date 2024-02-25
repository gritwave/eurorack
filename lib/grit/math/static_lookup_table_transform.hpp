#pragma once

#include <grit/math/remap.hpp>
#include <grit/math/static_lookup_table.hpp>

#include <etl/algorithm.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-math
template<etl::floating_point Float, etl::size_t Size>
struct StaticLookupTableTransform
{
    using ValueType = Float;
    using size_type  = etl::size_t;

    constexpr StaticLookupTableTransform() = default;

    template<etl::regular_invocable<Float> Func>
        requires(etl::same_as<etl::invoke_result_t<Func, Float>, Float>)
    explicit constexpr StaticLookupTableTransform(Func func, Float min, Float max)
    {
        initialize(func, min, max);
    }

    template<etl::regular_invocable<Float> Func>
        requires(etl::same_as<etl::invoke_result_t<Func, Float>, Float>)
    constexpr auto initialize(Func func, Float min, Float max) -> void
    {
        _min    = min;
        _max    = max;
        _scaler = static_cast<Float>(Size - 1) / (max - min);
        _offset = -min * _scaler;

        _lut.initialize([func, min, max](size_t i) {
            auto idx = remap(Float(i), Float(0), Float(Size - 1), min, max);
            return func(etl::clamp(idx, min, max));
        });
    }

    [[nodiscard]] constexpr auto atUnchecked(Float value) const -> Float
    {
        auto index = _scaler * value + _offset;
        return _lut[index];
    }

    [[nodiscard]] constexpr auto at(Float value) const -> Float { return atUnchecked(etl::clamp(value, _min, _max)); }

    [[nodiscard]] constexpr auto operator[](Float value) const -> Float { return at(value); }

    [[nodiscard]] constexpr auto operator()(Float value) const -> Float { return at(value); }

    [[nodiscard]] static constexpr auto size() -> etl::size_t { return Size; }

private:
    StaticLookupTable<Float, Size> _lut{};
    Float _min{};
    Float _max{};
    Float _scaler{};
    Float _offset{};
};

}  // namespace grit
