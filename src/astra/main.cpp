#include <grit/audio/container.hpp>
#include <grit/math/remap.hpp>
#include <grit/unit/decibel.hpp>

#include <daisy_patch_sm.h>

namespace {
namespace astra {

constexpr auto blockSize  = 16U;
constexpr auto sampleRate = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};

auto audioCallback(
    daisy::AudioHandle::InterleavingInputBuffer in,
    daisy::AudioHandle::InterleavingOutputBuffer out,
    size_t size
) -> void
{
    patch.ProcessAllControls();

    auto const input  = grit::StereoBlock<float const>{in, size};
    auto const output = grit::StereoBlock<float>{out, size};

    auto const gainLeftKnob  = patch.GetAdcValue(daisy::patch_sm::CV_1);
    auto const gainRightKnob = patch.GetAdcValue(daisy::patch_sm::CV_2);

    auto const gainLeft  = grit::fromDecibels(grit::remap(gainLeftKnob, -30.0F, 6.0F));
    auto const gainRight = grit::fromDecibels(grit::remap(gainRightKnob, -30.0F, 6.0F));

    for (size_t i = 0; i < size; ++i) {
        auto const inLeft  = input(0, i);
        auto const inRight = input(1, i);

        auto const leftGained  = inLeft * gainLeft;
        auto const rightGained = inRight * gainRight;

        output(0, i) = leftGained + rightGained;
        output(1, i) = leftGained + rightGained;
    }
}

}  // namespace astra

}  // namespace

auto main() -> int
{
    using namespace astra;

    patch.Init();
    patch.SetAudioSampleRate(sampleRate);
    patch.SetAudioBlockSize(blockSize);
    patch.StartAudio(audioCallback);

    while (true) {}
}
