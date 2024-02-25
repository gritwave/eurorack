#pragma once

namespace grit {

/// \ingroup grit-audio-stereo
template<typename Float>
struct MidSideFrame
{
    using SampleType = Float;

    Float mid{};
    Float side{};
};

}  // namespace grit
