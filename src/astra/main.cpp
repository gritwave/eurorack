#include <gw/audio/unit/decibel.hpp>
#include <gw/math/range.hpp>

#include <daisy_patch_sm.h>

namespace {
namespace astra {

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    auto const gainLeftKnob  = patch.GetAdcValue(daisy::patch_sm::CV_1);
    auto const gainRightKnob = patch.GetAdcValue(daisy::patch_sm::CV_2);

    auto const gainLeft  = gw::fromDecibels(gw::mapToRange(gainLeftKnob, -48.0F, 6.0F));
    auto const gainRight = gw::fromDecibels(gw::mapToRange(gainRightKnob, -48.0F, 6.0F));

    for (size_t i = 0; i < size; ++i) {
        auto const inLeft  = IN_L[i];
        auto const inRight = IN_R[i];

        auto const leftGained  = inLeft * gainLeft;
        auto const rightGained = inRight * gainRight;

        OUT_L[i] = leftGained + rightGained;
        OUT_R[i] = leftGained + rightGained;
    }
}

}  // namespace astra

}  // namespace

auto main() -> int
{
    using namespace astra;

    patch.Init();
    patch.SetAudioSampleRate(SAMPLE_RATE);
    patch.SetAudioBlockSize(BLOCK_SIZE);
    patch.StartAudio(audioCallback);

    while (true) {}
}
