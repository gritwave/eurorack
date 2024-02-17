#pragma once

#include <grit/eurorack/hades.hpp>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_extra/juce_gui_extra.h>

struct PluginProcessor final
    : juce::AudioProcessor
    , juce::AudioProcessorValueTreeState::Listener
{
    PluginProcessor();
    ~PluginProcessor() override;

    auto getName() const -> juce::String const override;

    auto prepareToPlay(double sampleRate, int samplesPerBlock) -> void override;
    auto processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) -> void override;
    using juce::AudioProcessor::processBlock;
    auto releaseResources() -> void override;

    auto isBusesLayoutSupported(BusesLayout const& layouts) const -> bool override;

    auto createEditor() -> juce::AudioProcessorEditor* override;
    auto hasEditor() const -> bool override;

    auto acceptsMidi() const -> bool override;
    auto producesMidi() const -> bool override;
    auto isMidiEffect() const -> bool override;
    auto getTailLengthSeconds() const -> double override;

    auto getNumPrograms() -> int override;
    auto getCurrentProgram() -> int override;
    void setCurrentProgram(int index) override;
    auto getProgramName(int index) -> juce::String const override;
    void changeProgramName(int index, juce::String const& newName) override;

    auto getStateInformation(juce::MemoryBlock& destData) -> void override;
    auto setStateInformation(void const* data, int sizeInBytes) -> void override;

    auto parameterChanged(juce::String const& parameterID, float newValue) -> void override;

    auto getState() noexcept -> juce::AudioProcessorValueTreeState&;
    auto getState() const noexcept -> juce::AudioProcessorValueTreeState const&;

    auto triggerNext() -> void { _next.store(true); }

private:
    juce::UndoManager _undoManager{};
    juce::AudioProcessorValueTreeState _valueTree;

    std::vector<float> _buffer{};
    std::unique_ptr<grit::Hades> _hades{nullptr};
    std::atomic<bool> _next{false};

    juce::AudioParameterFloat& _cv1;
    juce::AudioParameterFloat& _cv2;
    juce::AudioParameterFloat& _cv3;
    juce::AudioParameterFloat& _cv4;
    juce::AudioParameterFloat& _cv5;
    juce::AudioParameterFloat& _cv6;
    juce::AudioParameterFloat& _cv7;
    juce::AudioParameterFloat& _cv8;
};
