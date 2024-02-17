#include <grit/eurorack/hades.hpp>

#include <daisy_patch_sm.h>

namespace amp {

struct Amp
{
    enum struct Mode
    {
        Fire,
        Grind,
    };

    struct ControlInput
    {
        Mode mode{Mode::Fire};

        float gainKnob{0};
        float toneKnob{0};
        float outputKnob{0};
        float mixKnob{0};

        float gainCV{0};
        float toneCV{0};
        float outputCV{0};
        float mixCV{0};
    };

    Amp() = default;

    auto prepare(float sampleRate, etl::size_t blockSize) -> void
    {
        auto const blockRate = sampleRate / static_cast<float>(blockSize);

        _gainKnob.setSampleRate(blockRate);
        _toneKnob.setSampleRate(blockRate);
        _outputKnob.setSampleRate(blockRate);
        _mixKnob.setSampleRate(blockRate);
        _gainCV.setSampleRate(blockRate);
        _toneCV.setSampleRate(blockRate);
        _outputCV.setSampleRate(blockRate);
        _mixCV.setSampleRate(blockRate);

        _channels[0].setSampleRate(sampleRate);
        _channels[1].setSampleRate(sampleRate);
    }

    [[nodiscard]] auto process(grit::StereoBlock<float> const& buffer, ControlInput const& inputs) -> void
    {
        auto const gainKnob   = _gainKnob.process(inputs.gainKnob);
        auto const toneKnob   = _toneKnob.process(inputs.toneKnob);
        auto const outputKnob = _outputKnob.process(inputs.outputKnob);
        auto const mixKnob    = _mixKnob.process(inputs.mixKnob);

        auto const gainCV   = _gainCV.process(inputs.gainCV);
        auto const toneCV   = _toneCV.process(inputs.toneCV);
        auto const outputCV = _outputCV.process(inputs.outputCV);
        auto const mixCV    = _mixCV.process(inputs.mixCV);

        auto const parameter = Channel::Parameter{
            .mode   = inputs.mode,
            .gain   = etl::clamp(gainKnob + gainCV, 0.0F, 1.0F),
            .tone   = etl::clamp(toneKnob + toneCV, 0.0F, 1.0F),
            .output = etl::clamp(outputKnob + outputCV, 0.0F, 1.0F),
            .mix    = etl::clamp(mixKnob + mixCV, 0.0F, 1.0F),
        };

        for (auto& channel : _channels) {
            channel.setParameter(parameter);
        }

        for (auto i = size_t(0); i < buffer.extent(1); ++i) {
            buffer(0, i) = etl::invoke(_channels[0], buffer(0, i));
            buffer(1, i) = etl::invoke(_channels[1], buffer(1, i));
        }
    }

private:
    struct Channel
    {
        struct Parameter
        {
            Mode mode{Mode::Fire};
            float gain{0};
            float tone{0};
            float output{0};
            float mix{0};
        };

        Channel() = default;

        auto setParameter(Parameter const& parameter) -> void
        {
            if (parameter.mode != _mode) {
                _fire.reset();
                _grind.reset();
            }

            _mode = parameter.mode;

            if (parameter.mode == Mode::Fire) {
                _fire.setParameter({parameter.gain, parameter.tone, parameter.output, parameter.mix});
            } else {
                _grind.setParameter({parameter.gain, parameter.tone, parameter.output, parameter.mix});
            }
        }

        auto setSampleRate(float sampleRate) -> void
        {
            _fire.setSampleRate(sampleRate);
            _grind.setSampleRate(sampleRate);
        }

        [[nodiscard]] auto operator()(float sample) -> float
        {
            if (_mode == Mode::Fire) {
                return _fire(sample);
            } else {
                return _grind(sample);
            }
        }

    private:
        Mode _mode{Mode::Fire};

        grit::AirWindowsFireAmp<float> _fire{};
        grit::AirWindowsGrindAmp<float> _grind{};
    };

    grit::DynamicSmoothing<float> _gainKnob{};
    grit::DynamicSmoothing<float> _toneKnob{};
    grit::DynamicSmoothing<float> _outputKnob{};
    grit::DynamicSmoothing<float> _mixKnob{};
    grit::DynamicSmoothing<float> _gainCV{};
    grit::DynamicSmoothing<float> _toneCV{};
    grit::DynamicSmoothing<float> _outputCV{};
    grit::DynamicSmoothing<float> _mixCV{};

    etl::array<Channel, 2> _channels{};
};

static constexpr auto blockSize  = 32U;
static constexpr auto sampleRate = 96'000.0F;

auto processor = Amp{};
auto patch     = daisy::patch_sm::DaisyPatchSM{};
auto button    = daisy::Switch{};
auto toggle    = daisy::Switch{};
auto buffer    = etl::array<float, blockSize * 2U>{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();
    button.Debounce();
    toggle.Debounce();
    patch.SetLed(button.Pressed());

    // Copy to interleaved
    auto block = grit::StereoBlock<float>{buffer.data(), size};
    for (auto i{0U}; i < size; ++i) {
        block(0, i) = in[0][i];
        block(1, i) = in[1][i];
    }

    auto const controls = Amp::ControlInput{
        .mode       = toggle.Pressed() ? Amp::Mode::Fire : Amp::Mode::Grind,
        .gainKnob   = patch.GetAdcValue(daisy::patch_sm::CV_1),
        .toneKnob   = patch.GetAdcValue(daisy::patch_sm::CV_2),
        .outputKnob = patch.GetAdcValue(daisy::patch_sm::CV_3),
        .mixKnob    = patch.GetAdcValue(daisy::patch_sm::CV_4),
        .gainCV     = patch.GetAdcValue(daisy::patch_sm::CV_5),
        .toneCV     = patch.GetAdcValue(daisy::patch_sm::CV_6),
        .outputCV   = patch.GetAdcValue(daisy::patch_sm::CV_7),
        .mixCV      = patch.GetAdcValue(daisy::patch_sm::CV_8),
    };

    processor.process(block, controls);

    // Copy from interleaved
    for (auto i{0U}; i < size; ++i) {
        out[0][i] = block(0, i);
        out[1][i] = block(1, i);
    }
}

}  // namespace amp

auto main() -> int
{
    amp::patch.Init();
    amp::button.Init(amp::patch.B7);
    amp::toggle.Init(amp::patch.B8);

    amp::processor.prepare(amp::sampleRate, amp::blockSize);

    amp::patch.SetAudioSampleRate(amp::sampleRate);
    amp::patch.SetAudioBlockSize(amp::blockSize);
    amp::patch.StartAudio(amp::audioCallback);

    while (true) {}
}
