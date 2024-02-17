#include <grit/eurorack/amp.hpp>

#include <daisy_patch_sm.h>

namespace amp {

static constexpr auto blockSize  = 32U;
static constexpr auto sampleRate = 96'000.0F;

auto processor = grit::Amp{};
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

    auto const controls = grit::Amp::ControlInput{
        .mode       = toggle.Pressed() ? grit::Amp::Mode::Fire : grit::Amp::Mode::Grind,
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
