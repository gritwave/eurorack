#include <mc/audio/dynamic/compressor.hpp>
#include <mc/audio/dynamic/envelope_follower.hpp>
#include <mc/audio/noise/white_noise.hpp>
#include <mc/audio/waveshape/wave_shaper.hpp>

#include <etl/array.hpp>
#include <etl/functional.hpp>
#include <etl/span.hpp>

#include <daisy_patch_sm.h>

namespace hades
{

struct Channel
{
    Channel() = default;

    auto prepare(float sampleRate) -> void
    {
        _envelopeFollower.prepare(sampleRate);
        _compressor.prepare(sampleRate);
    }

    [[nodiscard]] auto processSample(float sample) -> float
    {
        auto const env     = _envelopeFollower.processSample(sample);
        auto const noise   = _noise.processSample();
        auto const noisy   = sample + noise * env;
        auto const distOut = _waveShaper.processSample(noisy);
        return _compressor.processSample(distOut, distOut);
    }

private:
    mc::EnvelopeFollower<float> _envelopeFollower{};
    mc::WhiteNoise<float> _noise{};
    mc::WaveShaper<float> _waveShaper{std::tanh};
    mc::Compressor<float> _compressor;
};

template<size_t BlockSize>
struct ProcessContext
{
    etl::array<etl::span<float const, BlockSize>, 2> input;
    etl::array<etl::span<float, BlockSize>, 2> output;
};

struct Hades
{
    Hades() = default;

    auto prepare(float sampleRate) -> void
    {
        _channels[0].prepare(sampleRate);
        _channels[1].prepare(sampleRate);
    }

    template<size_t BlockSize>
    [[nodiscard]] auto processBlock(ProcessContext<BlockSize> const& context) -> void
    {
        for (size_t i = 0; i < BlockSize; ++i)
        {
            auto const left  = context.input[0][i];
            auto const right = context.input[1][i];

            context.output[0][i] = _channels[0].processSample(left);
            context.output[1][i] = _channels[1].processSample(right);
        }
    }

private:
    etl::array<Channel, 2> _channels{};
};

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};
auto hades = Hades{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    // GATE DIGITAL LOGIC
    auto const gate1   = patch.gate_in_1.State();
    auto const gate2   = patch.gate_in_2.State();
    auto const gateOut = gate1 != gate2;
    dsy_gpio_write(&patch.gate_out_1, gateOut);
    dsy_gpio_write(&patch.gate_out_2, not gateOut);

    auto const leftIn   = etl::span<float const, BLOCK_SIZE>{etl::addressof(IN_L[0]), BLOCK_SIZE};
    auto const rightIn  = etl::span<float const, BLOCK_SIZE>{etl::addressof(IN_R[0]), BLOCK_SIZE};
    auto const leftOut  = etl::span<float, BLOCK_SIZE>{etl::addressof(OUT_L[0]), BLOCK_SIZE};
    auto const rightOut = etl::span<float, BLOCK_SIZE>{etl::addressof(OUT_R[0]), BLOCK_SIZE};

    hades.processBlock(ProcessContext<BLOCK_SIZE>{
        .input  = {leftIn, rightIn},
        .output = {leftOut, rightOut},
    });
}

}  // namespace hades

auto main() -> int
{
    hades::patch.Init();
    hades::patch.SetAudioSampleRate(hades::SAMPLE_RATE);
    hades::patch.SetAudioBlockSize(hades::BLOCK_SIZE);
    hades::hades.prepare(hades::SAMPLE_RATE);
    hades::patch.StartAudio(hades::audioCallback);
    while (true) {}
}
