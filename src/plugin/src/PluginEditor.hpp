#pragma once

#include "PluginProcessor.hpp"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

struct PluginEditor final : juce::AudioProcessorEditor
{
    explicit PluginEditor(PluginProcessor& p);
    ~PluginEditor() noexcept override;

    auto paint(juce::Graphics& g) -> void override;
    auto resized() -> void override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::OwnedArray<juce::Slider> _sliders;
    juce::OwnedArray<juce::Label> _labels;
    juce::OwnedArray<SliderAttachment> _attachments;
    juce::TextButton _next{"Next"};

    juce::SharedResourcePointer<juce::TooltipWindow> _tooltipWindow;
};
