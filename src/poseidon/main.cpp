#include <grit/eurorack/poseidon.hpp>

#include <etl/linalg.hpp>

#include <daisy_patch_sm.h>

namespace poseidon {

static constexpr auto blockSize  = 32U;
static constexpr auto sampleRate = 96'000.0F;

auto processor = grit::Poseidon{};
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

    if (button.FallingEdge()) {
        if (toggle.Pressed()) {
            processor.nextDistortionAlgorithm();
        } else {
            processor.nextTextureAlgorithm();
        }
    }

    auto const controls = grit::Poseidon::ControlInput{
        .textureKnob    = patch.GetAdcValue(daisy::patch_sm::CV_1),
        .morphKnob      = patch.GetAdcValue(daisy::patch_sm::CV_2),
        .ampKnob        = patch.GetAdcValue(daisy::patch_sm::CV_3),
        .compressorKnob = patch.GetAdcValue(daisy::patch_sm::CV_4),
        .morphCV        = patch.GetAdcValue(daisy::patch_sm::CV_5),
        .sideChainCV    = patch.GetAdcValue(daisy::patch_sm::CV_6),
        .attackCV       = patch.GetAdcValue(daisy::patch_sm::CV_7),
        .releaseCV      = patch.GetAdcValue(daisy::patch_sm::CV_8),
        .gate1          = patch.gate_in_1.State(),
        .gate2          = patch.gate_in_2.State(),
    };

    auto const input  = grit::StereoBlock<float const>{in, size};
    auto const output = grit::StereoBlock<float>{out, size};
    etl::linalg::copy(input, output);

    auto const cvOut = processor.process(output, controls);

    patch.WriteCvOut(daisy::patch_sm::CV_OUT_BOTH, cvOut.envelope * 5.0F);
    dsy_gpio_write(&patch.gate_out_1, static_cast<uint8_t>(cvOut.gate1));
    dsy_gpio_write(&patch.gate_out_2, static_cast<uint8_t>(cvOut.gate2));
}

}  // namespace poseidon

auto main() -> int
{
    poseidon::patch.Init();
    poseidon::button.Init(daisy::patch_sm::DaisyPatchSM::B7);
    poseidon::toggle.Init(daisy::patch_sm::DaisyPatchSM::B8);

    poseidon::processor.prepare(poseidon::sampleRate, poseidon::blockSize);

    poseidon::patch.SetAudioSampleRate(poseidon::sampleRate);
    poseidon::patch.SetAudioBlockSize(poseidon::blockSize);
    poseidon::patch.StartAudio(poseidon::audioCallback);

    while (true) {}
}
