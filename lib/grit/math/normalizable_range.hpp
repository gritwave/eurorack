#pragma once

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \brief Mapping between a range of values and a normalized 0 to 1 range.
/// \warning This is **not** a drop-in replacement for juce::NormalisableRange. The 3rd
///          constructor parameter is the skew midpoint and symmetric skew is **not** supported.
/// \ingroup grit-math
template<etl::floating_point Float>
struct NormalizableRange
{
    using value_type = Float;

    constexpr NormalizableRange() = default;

    constexpr NormalizableRange(Float start, Float end) noexcept : _start{start}, _end{end} {}

    constexpr NormalizableRange(Float start, Float end, Float midpoint) noexcept
        : _start{start}
        , _end{end}
        , _skew{skewForMidpoint(start, end, midpoint)}
    {}

    [[nodiscard]] constexpr auto getStart() const -> Float { return _start; }

    [[nodiscard]] constexpr auto getEnd() const -> Float { return _end; }

    [[nodiscard]] constexpr auto from0to1(Float proportion) const -> Float
    {
        proportion = etl::clamp(proportion, Float(0), Float(1));

        if ((_skew != Float(1)) and (proportion > Float(0))) {
            proportion = etl::exp(etl::log(proportion) / _skew);
        }

        return _start + (_end - _start) * proportion;
    }

    [[nodiscard]] constexpr auto to0to1(Float value) const -> Float
    {
        auto const proportion = etl::clamp((value - _start) / (_end - _start), Float(0), Float(1));

        if (_skew == Float(1)) {
            return proportion;
        }

        return etl::pow(proportion, _skew);
    }

private:
    [[nodiscard]] static constexpr auto skewForMidpoint(Float start, Float end, Float midpoint) -> Float
    {
        return etl::log(Float(0.5)) / etl::log((midpoint - start) / (end - start));
    }

    Float _start{0};
    Float _end{0};
    Float _skew{1};
};

}  // namespace grit
