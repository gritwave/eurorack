#pragma once

namespace grit {

/// \ingroup grit-audio-stereo
template<typename SampleType>
struct MidSideFrame
{
    using value_type = SampleType;

    SampleType mid{};
    SampleType side{};
};

}  // namespace grit
