#include <daisy_patch_sm.h>

namespace astra
{

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    for (size_t i = 0; i < size; ++i)
    {
        auto const left  = IN_L[i];
        auto const right = IN_R[i];

        OUT_L[i] = left;
        OUT_R[i] = right;
    }
}

}  // namespace astra

auto main() -> int
{
    using namespace astra;

    patch.Init();
    patch.SetAudioSampleRate(SAMPLE_RATE);
    patch.SetAudioBlockSize(BLOCK_SIZE);
    patch.StartAudio(audioCallback);

    while (true) {}
}
