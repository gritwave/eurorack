#include <gw/eurorack/hades.hpp>

#include <daisy_patch_sm.h>

namespace hades
{

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};
auto hades = gw::Hades{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    auto const leftIn   = etl::span<float const>{etl::addressof(IN_L[0]), BLOCK_SIZE};
    auto const rightIn  = etl::span<float const>{etl::addressof(IN_R[0]), BLOCK_SIZE};
    auto const leftOut  = etl::span<float>{etl::addressof(OUT_L[0]), BLOCK_SIZE};
    auto const rightOut = etl::span<float>{etl::addressof(OUT_R[0]), BLOCK_SIZE};
    auto const context  = gw::Hades::Buffers{
         .input  = {leftIn, rightIn},
         .output = {leftOut, rightOut},
    };

    auto const inputs = gw::Hades::ControlInputs{
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
    dsy_gpio_write(&patch.gate_out_1, outputs.gate1);
    dsy_gpio_write(&patch.gate_out_2, outputs.gate2);
}

}  // namespace hades

auto main() -> int
{
    hades::patch.Init();
    hades::patch.SetAudioSampleRate(hades::SAMPLE_RATE);
    hades::patch.SetAudioBlockSize(hades::BLOCK_SIZE);
    hades::hades.prepare(hades::SAMPLE_RATE, hades::BLOCK_SIZE);
    hades::patch.StartAudio(hades::audioCallback);
    while (true) {}
}
