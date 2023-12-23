#include <grit/eurorack/hades.hpp>

#include <daisy_patch_sm.h>

namespace hades {

static constexpr auto block_size  = 16U;
static constexpr auto sample_rate = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};
auto hades = grit::hades{};

auto audio_callback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t /*size*/) -> void
{
    patch.ProcessAllControls();

    auto const left_in   = etl::span<float const>{etl::addressof(IN_L[0]), block_size};
    auto const right_in  = etl::span<float const>{etl::addressof(IN_R[0]), block_size};
    auto const left_out  = etl::span<float>{etl::addressof(OUT_L[0]), block_size};
    auto const right_out = etl::span<float>{etl::addressof(OUT_R[0]), block_size};
    auto const context   = grit::hades::buffers{
          .input  = { left_in,  right_in},
          .output = {left_out, right_out},
    };

    auto const inputs = grit::hades::control_inputs{
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

    auto const outputs = hades.process_block(context, inputs);
    patch.WriteCvOut(daisy::patch_sm::CV_OUT_1, outputs.envelope * 5.0F);
    patch.WriteCvOut(daisy::patch_sm::CV_OUT_2, outputs.envelope * 5.0F);
    dsy_gpio_write(&patch.gate_out_1, static_cast<uint8_t>(outputs.gate1));
    dsy_gpio_write(&patch.gate_out_2, static_cast<uint8_t>(outputs.gate2));
}

}  // namespace hades

auto main() -> int
{
    hades::patch.Init();
    hades::patch.SetAudioSampleRate(hades::sample_rate);
    hades::patch.SetAudioBlockSize(hades::block_size);
    hades::hades.prepare(hades::sample_rate, hades::block_size);
    hades::patch.StartAudio(hades::audio_callback);
    while (true) {}
}
