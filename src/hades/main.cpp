#include <grit/eurorack/hades.hpp>

#include <daisy_patch_sm.h>

namespace hades {

static constexpr auto blockSize  = 32U;
static constexpr auto sampleRate = 96'000.0F;

auto processor = grit::Hades{};
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

    if (button.FallingEdge()) {
        if (toggle.Pressed()) {
            processor.nextDistortionAlgorithm();
        } else {
            processor.nextTextureAlgorithm();
        }
    }

    // Copy to interleaved
    auto io = grit::StereoBlock<float>{buffer.data(), size};
    for (auto i{0U}; i < size; ++i) {
        io(0, i) = in[0][i];
        io(1, i) = in[1][i];
    }

    auto const controls = grit::Hades::ControlInput{
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

    auto const outputs = processor.process(io, controls);

    // Copy from interleaved
    for (auto i{0U}; i < size; ++i) {
        out[0][i] = io(0, i);
        out[1][i] = io(1, i);
    }

    patch.WriteCvOut(daisy::patch_sm::CV_OUT_BOTH, outputs.envelope * 5.0F);
    dsy_gpio_write(&patch.gate_out_1, static_cast<uint8_t>(outputs.gate1));
    dsy_gpio_write(&patch.gate_out_2, static_cast<uint8_t>(outputs.gate2));
}

}  // namespace hades

auto main() -> int
{
    hades::patch.Init();
    hades::button.Init(daisy::patch_sm::DaisyPatchSM::B7);
    hades::toggle.Init(daisy::patch_sm::DaisyPatchSM::B8);

    hades::processor.prepare(hades::sampleRate, hades::blockSize);

    hades::patch.SetAudioSampleRate(hades::sampleRate);
    hades::patch.SetAudioBlockSize(hades::blockSize);
    hades::patch.StartAudio(hades::audioCallback);

    while (true) {}
}
