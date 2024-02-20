#include "PluginParameter.hpp"

namespace gritrack {

auto createParameters() -> juce::AudioProcessorValueTreeState::ParameterLayout
{
    auto const normalized = juce::NormalisableRange(0.0F, 1.0F);

    return {
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamID::cv1, 1}, "CV-1", normalized, 0.0F),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamID::cv2, 1}, "CV-2", normalized, 0.0F),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamID::cv3, 1}, "CV-3", normalized, 0.0F),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamID::cv4, 1}, "CV-4", normalized, 0.0F),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamID::cv5, 1}, "CV-5", normalized, 0.0F),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamID::cv6, 1}, "CV-6", normalized, 0.0F),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamID::cv7, 1}, "CV-7", normalized, 0.0F),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ParamID::cv8, 1}, "CV-8", normalized, 0.0F),
    };
}

}  // namespace gritrack
