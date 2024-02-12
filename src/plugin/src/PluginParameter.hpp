#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

namespace gritrack {

struct ParamID
{
    static constexpr char const* cv1 = "cv1";
    static constexpr char const* cv2 = "cv2";
    static constexpr char const* cv3 = "cv3";
    static constexpr char const* cv4 = "cv4";
    static constexpr char const* cv5 = "cv5";
    static constexpr char const* cv6 = "cv6";
    static constexpr char const* cv7 = "cv7";
    static constexpr char const* cv8 = "cv8";
};

auto createParameters() -> juce::AudioProcessorValueTreeState::ParameterLayout;

}  // namespace gritrack
