#pragma once

#include <grit/unit/decibel.hpp>

namespace grit {

/// \ingroup grit-audio-dynamic
template<etl::floating_point Float>
struct PeakLevelDetector
{
    PeakLevelDetector() = default;

    [[nodiscard]] constexpr auto operator()(Float x) -> Float { return toDecibels(x); }
};

}  // namespace grit
