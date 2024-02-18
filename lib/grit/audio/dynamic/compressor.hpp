#pragma once

#include <grit/audio/dynamic/dynamic.hpp>
#include <grit/audio/dynamic/gain_computer.hpp>
#include <grit/audio/dynamic/level_detector.hpp>
#include <grit/audio/envelope/envelope_follower.hpp>

namespace grit {

/// \ingroup grit-audio-dynamic
template<etl::floating_point Float>
using HardKneeCompressor
    = Dynamic<Float, PeakLevelDetector<Float>, HardKneeGainComputer<Float>, EnvelopeFollower<Float>>;

/// \ingroup grit-audio-dynamic
template<etl::floating_point Float>
using SoftKneeCompressor
    = Dynamic<Float, PeakLevelDetector<Float>, SoftKneeGainComputer<Float>, EnvelopeFollower<Float>>;

}  // namespace grit
