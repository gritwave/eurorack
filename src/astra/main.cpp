#include <grit/audio/unit/decibel.hpp>
#include <grit/math/range.hpp>

#include <daisy_patch_sm.h>

namespace {
namespace astra {

constexpr auto block_size  = 16U;
constexpr auto sample_rate = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};

auto audio_callback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    auto const gain_left_knob  = patch.GetAdcValue(daisy::patch_sm::CV_1);
    auto const gain_right_knob = patch.GetAdcValue(daisy::patch_sm::CV_2);

    auto const gain_left  = grit::from_decibels(grit::map_to_range(gain_left_knob, -30.0F, 6.0F));
    auto const gain_right = grit::from_decibels(grit::map_to_range(gain_right_knob, -30.0F, 6.0F));

    for (size_t i = 0; i < size; ++i) {
        auto const in_left  = IN_L[i];
        auto const in_right = IN_R[i];

        auto const left_gained  = in_left * gain_left;
        auto const right_gained = in_right * gain_right;

        OUT_L[i] = left_gained + right_gained;
        OUT_R[i] = left_gained + right_gained;
    }
}

}  // namespace astra

}  // namespace

auto main() -> int
{
    using namespace astra;

    patch.Init();
    patch.SetAudioSampleRate(sample_rate);
    patch.SetAudioBlockSize(block_size);
    patch.StartAudio(audio_callback);

    while (true) {}
}
