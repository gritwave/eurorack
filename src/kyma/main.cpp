#include <grit/eurorack/kyma.hpp>

#include <daisy_patch_sm.h>

namespace kyma {

static constexpr auto blockSize  = 16U;
static constexpr auto sampleRate = 96'000.0F;

auto patch     = daisy::patch_sm::DaisyPatchSM{};
auto toggle    = daisy::Switch{};
auto button    = daisy::Switch{};
auto processor = grit::Kyma{};

auto audioCallback(
    daisy::AudioHandle::InterleavingInputBuffer in,
    daisy::AudioHandle::InterleavingOutputBuffer out,
    size_t size
) -> void
{
    patch.ProcessAllControls();
    toggle.Debounce();
    button.Debounce();
    patch.SetLed(not toggle.Pressed());

    auto const controls = grit::Kyma::ControlInput{
        .pitchKnob   = patch.GetAdcValue(daisy::patch_sm::CV_1),
        .morphKnob   = patch.GetAdcValue(daisy::patch_sm::CV_3),
        .attackKnob  = patch.GetAdcValue(daisy::patch_sm::CV_2),
        .releaseKnob = patch.GetAdcValue(daisy::patch_sm::CV_4),
        .vOctCV      = patch.GetAdcValue(daisy::patch_sm::CV_5),
        .morphCV     = patch.GetAdcValue(daisy::patch_sm::CV_6),
        .subGainCV   = patch.GetAdcValue(daisy::patch_sm::CV_7),
        .subMorphCV  = patch.GetAdcValue(daisy::patch_sm::CV_8),
        .gate        = patch.gate_in_1.State() or button.Pressed(),
        .subShift    = toggle.Pressed(),
    };

    auto const block = grit::StereoBlock<float>{out, size};
    auto const env   = processor.process(block, controls);
    patch.WriteCvOut(daisy::patch_sm::CV_OUT_2, env * 5.0F);
}

}  // namespace kyma

auto main() -> int
{
    kyma::patch.Init();

    kyma::toggle.Init(daisy::patch_sm::DaisyPatchSM::B8);
    kyma::button.Init(daisy::patch_sm::DaisyPatchSM::B7);

    kyma::processor.prepare(kyma::sampleRate, kyma::blockSize);

    kyma::patch.SetAudioSampleRate(kyma::sampleRate);
    kyma::patch.SetAudioBlockSize(kyma::blockSize);
    kyma::patch.StartAudio(kyma::audioCallback);

    while (true) {}
}
