#include "PluginEditor.hpp"

#include "PluginParameter.hpp"

PluginEditor::PluginEditor(PluginProcessor& p) : AudioProcessorEditor(&p)
{
    _tooltipWindow->setMillisecondsBeforeTipAppears(750);

    auto addSlider = [this, &p](auto id) {
        auto const style   = juce::Slider::LinearHorizontal;
        auto const textBox = juce::Slider::TextBoxRight;
        auto* const slider = _sliders.add(std::make_unique<juce::Slider>(style, textBox));

        auto* const label = _labels.add(std::make_unique<juce::Label>(id, id));
        label->setJustificationType(juce::Justification::centred);

        _attachments.add(std::make_unique<SliderAttachment>(p.getState(), id, *slider));

        addAndMakeVisible(slider);
        addAndMakeVisible(label);
    };

    addSlider(gritrack::ParamID::cv1);
    addSlider(gritrack::ParamID::cv2);
    addSlider(gritrack::ParamID::cv3);
    addSlider(gritrack::ParamID::cv4);
    addSlider(gritrack::ParamID::cv5);
    addSlider(gritrack::ParamID::cv6);
    addSlider(gritrack::ParamID::cv7);
    addSlider(gritrack::ParamID::cv8);

    _next.onClick = [&p] { p.triggerNext(); };
    addAndMakeVisible(_next);

    setSize(600, 400);
}

PluginEditor::~PluginEditor() noexcept = default;

auto PluginEditor::paint(juce::Graphics& g) -> void
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

auto PluginEditor::resized() -> void
{
    auto bounds = getLocalBounds();
    auto height = bounds.proportionOfHeight(1.0 / (_sliders.size() + 1));
    for (auto i{0}; i < _sliders.size(); ++i) {
        auto row = bounds.removeFromTop(height);
        _labels[i]->setBounds(row.removeFromLeft(row.proportionOfWidth(0.25)));
        _sliders[i]->setBounds(row);
    }

    _next.setBounds(bounds);
}
