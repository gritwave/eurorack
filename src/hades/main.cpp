#include <grit/eurorack/hades.hpp>

#include <daisy_patch_sm.h>

namespace hades {

static constexpr auto blockSize  = 32U;
static constexpr auto sampleRate = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};
auto hades = grit::Hades{};

auto audioCallback(
    daisy::AudioHandle::InterleavingInputBuffer in,
    daisy::AudioHandle::InterleavingOutputBuffer out,
    size_t size
) -> void
{
    patch.ProcessAllControls();

    auto const context = grit::Hades::Buffer{
        .input  = grit::StereoBlock<float const>{ in, size},
        .output = grit::StereoBlock<float>{out, size},
    };

    auto const inputs = grit::Hades::ControlInput{
        .textureKnob    = patch.GetAdcValue(daisy::patch_sm::CV_1),
        .morphKnob      = patch.GetAdcValue(daisy::patch_sm::CV_3),
        .ampKnob        = patch.GetAdcValue(daisy::patch_sm::CV_2),
        .compressorKnob = patch.GetAdcValue(daisy::patch_sm::CV_4),
        .morphCV        = patch.GetAdcValue(daisy::patch_sm::CV_5),
        .sideChainCV    = patch.GetAdcValue(daisy::patch_sm::CV_6),
        .attackCV       = patch.GetAdcValue(daisy::patch_sm::CV_7),
        .releaseCV      = patch.GetAdcValue(daisy::patch_sm::CV_8),
        .gate1          = patch.gate_in_1.State(),
        .gate2          = patch.gate_in_2.State(),
    };

    auto const outputs = hades.processBlock(context, inputs);
    patch.WriteCvOut(daisy::patch_sm::CV_OUT_1, outputs.envelope * 5.0F);
    patch.WriteCvOut(daisy::patch_sm::CV_OUT_2, outputs.envelope * 5.0F);
    dsy_gpio_write(&patch.gate_out_1, static_cast<uint8_t>(outputs.gate1));
    dsy_gpio_write(&patch.gate_out_2, static_cast<uint8_t>(outputs.gate2));
}

}  // namespace hades

auto main() -> int
{
    hades::patch.Init();
    hades::patch.SetAudioSampleRate(hades::sampleRate);
    hades::patch.SetAudioBlockSize(hades::blockSize);
    hades::hades.prepare(hades::sampleRate, hades::blockSize);
    hades::patch.StartAudio(hades::audioCallback);
    while (true) {}
}
