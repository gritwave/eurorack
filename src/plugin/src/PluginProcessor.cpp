
#include "PluginProcessor.hpp"

#include "PluginEditor.hpp"
#include "PluginParameter.hpp"

PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , _valueTree{*this, nullptr, juce::Identifier("Gritwave"), gritrack::createParameters()}
    , _cv1{*dynamic_cast<juce::AudioParameterFloat*>(_valueTree.getParameter(gritrack::ParamID::cv1))}
    , _cv2{*dynamic_cast<juce::AudioParameterFloat*>(_valueTree.getParameter(gritrack::ParamID::cv2))}
    , _cv3{*dynamic_cast<juce::AudioParameterFloat*>(_valueTree.getParameter(gritrack::ParamID::cv3))}
    , _cv4{*dynamic_cast<juce::AudioParameterFloat*>(_valueTree.getParameter(gritrack::ParamID::cv4))}
    , _cv5{*dynamic_cast<juce::AudioParameterFloat*>(_valueTree.getParameter(gritrack::ParamID::cv5))}
    , _cv6{*dynamic_cast<juce::AudioParameterFloat*>(_valueTree.getParameter(gritrack::ParamID::cv6))}
    , _cv7{*dynamic_cast<juce::AudioParameterFloat*>(_valueTree.getParameter(gritrack::ParamID::cv7))}
    , _cv8{*dynamic_cast<juce::AudioParameterFloat*>(_valueTree.getParameter(gritrack::ParamID::cv8))}
{}

PluginProcessor::~PluginProcessor() = default;

auto PluginProcessor::getName() const -> juce::String const { return JucePlugin_Name; }

auto PluginProcessor::acceptsMidi() const -> bool { return false; }

auto PluginProcessor::producesMidi() const -> bool { return false; }

auto PluginProcessor::isMidiEffect() const -> bool { return false; }

auto PluginProcessor::getTailLengthSeconds() const -> double { return 0.0; }

auto PluginProcessor::getNumPrograms() -> int { return 1; }

auto PluginProcessor::getCurrentProgram() -> int { return 0; }

auto PluginProcessor::setCurrentProgram(int index) -> void { juce::ignoreUnused(index); }

auto PluginProcessor::getProgramName(int index) -> juce::String const
{
    juce::ignoreUnused(index);
    return {};
}

auto PluginProcessor::changeProgramName(int index, juce::String const& newName) -> void
{
    juce::ignoreUnused(index, newName);
}

auto PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) -> void
{
    _hades = std::make_unique<grit::Hades>();
    _hades->prepare(sampleRate, static_cast<std::size_t>(samplesPerBlock));

    _buffer.resize(2U * static_cast<std::size_t>(samplesPerBlock));
}

auto PluginProcessor::releaseResources() -> void {}

auto PluginProcessor::isBusesLayoutSupported(BusesLayout const& layouts) const -> bool
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
        return false;
    }

    return true;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals const noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    auto io = grit::StereoBlock<float>{_buffer.data(), static_cast<etl::size_t>(buffer.getNumSamples())};
    for (auto i = 0; i < buffer.getNumSamples(); ++i) {
        io(0, i) = buffer.getSample(0, i);
        io(1, i) = buffer.getSample(1, i);
    }

    auto const controls = grit::Hades::ControlInput{
        .textureKnob    = _cv1,
        .morphKnob      = _cv2,
        .ampKnob        = _cv3,
        .compressorKnob = _cv4,
        .morphCV        = _cv5,
        .sideChainCV    = _cv6,
        .attackCV       = _cv7,
        .releaseCV      = _cv8,
        .gate1          = false,
        .gate2          = false,
    };

    auto const cv = _hades->process(io, controls);
    juce::ignoreUnused(cv);

    for (auto i = 0; i < buffer.getNumSamples(); ++i) {
        buffer.setSample(0, i, io(0, i));
        buffer.setSample(1, i, io(1, i));
    }
}

auto PluginProcessor::parameterChanged(juce::String const& parameterID, float newValue) -> void
{
    juce::ignoreUnused(newValue, parameterID);
}

auto PluginProcessor::hasEditor() const -> bool { return true; }

auto PluginProcessor::createEditor() -> juce::AudioProcessorEditor*
{
    return new juce::GenericAudioProcessorEditor(*this);
}

auto PluginProcessor::getStateInformation(juce::MemoryBlock& destData) -> void
{
    juce::MemoryOutputStream stream(destData, false);
    _valueTree.state.writeToStream(stream);
}

auto PluginProcessor::setStateInformation(void const* data, int sizeInBytes) -> void
{
    juce::ValueTree const tree = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));
    jassert(tree.isValid());
    if (tree.isValid()) {
        _valueTree.state = tree;
    }
}

auto PluginProcessor::getState() noexcept -> juce::AudioProcessorValueTreeState& { return _valueTree; }

auto PluginProcessor::getState() const noexcept -> juce::AudioProcessorValueTreeState const& { return _valueTree; }

// This creates new instances of the plugin..
auto JUCE_CALLTYPE createPluginFilter() -> juce::AudioProcessor* { return new PluginProcessor(); }
