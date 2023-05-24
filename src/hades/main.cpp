#include <mc/audio/waveshape/wave_shaper.hpp>

#include <etl/array.hpp>

#include <daisy_patch_sm.h>

namespace hades
{

struct Channel
{
    Channel() = default;

    auto prepare(float sampleRate) -> void { etl::ignore_unused(sampleRate); }
    [[nodiscard]] auto processSample(float sample) -> float { return _waveShaper.processSample(sample); }

private:
    mc::audio::WaveShaper<float> _waveShaper{std::tanh};
};

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto patch    = daisy::patch_sm::DaisyPatchSM{};
auto channels = etl::array<Channel, 2>{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    for (size_t i = 0; i < size; ++i)
    {
        auto const left  = IN_L[i];
        auto const right = IN_R[i];

        OUT_L[i] = channels[0].processSample(left);
        OUT_R[i] = channels[1].processSample(right);
    }
}

}  // namespace hades

auto main() -> int
{
    using namespace hades;

    patch.Init();
    patch.SetAudioSampleRate(SAMPLE_RATE);
    patch.SetAudioBlockSize(BLOCK_SIZE);

    channels[0].prepare(SAMPLE_RATE);
    channels[1].prepare(SAMPLE_RATE);

    patch.StartAudio(audioCallback);
    while (true) {}
}
