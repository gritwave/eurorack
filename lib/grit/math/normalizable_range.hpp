#pragma once

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>

namespace grit {

/// \ingroup grit-math
template<etl::floating_point Float>
struct NormalizableRange
{
    using value_type = Float;

    constexpr NormalizableRange() = default;

    constexpr NormalizableRange(Float start, Float end) noexcept : _start{start}, _end{end} {}

    constexpr NormalizableRange(Float start, Float end, Float center) noexcept
        : _start{start}
        , _end{end}
        , _skew{skewForCenter(start, end, center)}
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
    [[nodiscard]] static constexpr auto skewForCenter(Float start, Float end, Float center) -> Float
    {
        return etl::log(Float(0.5)) / etl::log((center - start) / (end - start));
    }

    Float _start{0};
    Float _end{0};
    Float _skew{1};
};

}  // namespace grit
