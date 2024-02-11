#pragma once

namespace grit {

template<typename SampleType>
struct MidSideFrame
{
    using value_type = SampleType;

    SampleType mid{};
    SampleType side{};
};

}  // namespace grit
