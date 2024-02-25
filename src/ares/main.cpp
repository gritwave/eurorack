#include <grit/eurorack/ares.hpp>

#include <etl/linalg.hpp>

#include <daisy_patch_sm.h>

namespace ares {

static constexpr auto blockSize  = 32U;
static constexpr auto sampleRate = 96'000.0F;

auto processor = grit::Ares{};
auto patch     = daisy::patch_sm::DaisyPatchSM{};
auto button    = daisy::Switch{};
auto toggle    = daisy::Switch{};

auto audioCallback(
    daisy::AudioHandle::InterleavingInputBuffer in,
    daisy::AudioHandle::InterleavingOutputBuffer out,
    size_t size
) -> void
{
    patch.ProcessAllControls();
    button.Debounce();
    toggle.Debounce();
    patch.SetLed(button.Pressed());

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

    auto const input  = grit::StereoBlock<float const>{in, size};
    auto const output = grit::StereoBlock<float>{out, size};
    etl::linalg::copy(input, output);

    processor.process(output, controls);
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
