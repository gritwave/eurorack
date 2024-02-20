#include <grit/eurorack/ares.hpp>

#include <daisy_patch_sm.h>

namespace ares {

static constexpr auto blockSize  = 32U;
static constexpr auto sampleRate = 96'000.0F;

auto processor = grit::Ares{};
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

    auto const controls = grit::Ares::ControlInput{
        .mode       = toggle.Pressed() ? grit::Ares::Mode::Fire : grit::Ares::Mode::Grind,
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

}  // namespace ares

auto main() -> int
{
    ares::patch.Init();
    ares::button.Init(ares::patch.B7);
    ares::toggle.Init(ares::patch.B8);

    ares::processor.prepare(ares::sampleRate, ares::blockSize);

    ares::patch.SetAudioSampleRate(ares::sampleRate);
    ares::patch.SetAudioBlockSize(ares::blockSize);
    ares::patch.StartAudio(ares::audioCallback);

    while (true) {}
}
