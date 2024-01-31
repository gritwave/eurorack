#include <grit/audio/unit/decibel.hpp>
#include <grit/math/range.hpp>

#include <daisy_patch_sm.h>

#include <etl/mdspan.hpp>

namespace {
namespace astra {

template<etl::floating_point Float>
using stereo_buffer = etl::mdspan<Float, etl::extents<size_t, 2, etl::dynamic_extent>, etl::layout_left>;

constexpr auto block_size  = 16U;
constexpr auto sample_rate = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};

auto audio_callback(
    daisy::AudioHandle::InterleavingInputBuffer in,
    daisy::AudioHandle::InterleavingOutputBuffer out,
    size_t size
) -> void
{
    patch.ProcessAllControls();

    auto const input  = stereo_buffer<float const>{in, size};
    auto const output = stereo_buffer<float>{out, size};

    auto const gain_left_knob  = patch.GetAdcValue(daisy::patch_sm::CV_1);
    auto const gain_right_knob = patch.GetAdcValue(daisy::patch_sm::CV_2);

    auto const gain_left  = grit::fromDecibels(grit::mapToRange(gain_left_knob, -30.0F, 6.0F));
    auto const gain_right = grit::fromDecibels(grit::mapToRange(gain_right_knob, -30.0F, 6.0F));

    for (size_t i = 0; i < size; ++i) {
        auto const in_left  = input(0, i);
        auto const in_right = input(1, i);

        auto const left_gained  = in_left * gain_left;
        auto const right_gained = in_right * gain_right;

        output(0, i) = left_gained + right_gained;
        output(1, i) = left_gained + right_gained;
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
