#include <mc/audio/dynamic/compressor.hpp>
#include <mc/audio/dynamic/envelope_follower.hpp>
#include <mc/audio/noise/white_noise.hpp>
#include <mc/audio/waveshape/wave_shaper.hpp>
#include <mc/math/dynamic_smoothing.hpp>

#include <etl/array.hpp>
#include <etl/functional.hpp>
#include <etl/span.hpp>

#include <daisy_patch_sm.h>

namespace hades
{

struct Channel
{
    struct Parameter
    {
        float texture{0.0F};
        float morph{0.0F};
        float amp{0.0F};
        float compressor{0.0F};
        float sideChain{0.0F};
        float attack{0.0F};
        float release{0.0F};
    };

    Channel() = default;

    auto setParameter(Parameter const& parameter) -> void { _parameter = parameter; }

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
    Parameter _parameter{};
    mc::EnvelopeFollower<float> _envelopeFollower{};
    mc::WhiteNoise<float> _noise{};
    mc::WaveShaper<float> _waveShaper{std::tanh};
    mc::Compressor<float> _compressor;
};

struct Hades
{
    struct ControlInputs
    {
        float textureKnob{0};
        float morphKnob{0};
        float ampKnob{0};
        float compressorKnob{0};
        float morphCV{0};
        float sideChainCV{0};
        float attackCV{0};
        float releaseCV{0};

        bool gate1{false};
        bool gate2{false};
    };

    struct ControlOutputs
    {
        float envelope{0};
        bool gate1{false};
        bool gate2{false};
    };

    template<size_t BlockSize>
    struct Buffers
    {
        etl::array<etl::span<float const, BlockSize>, 2> input;
        etl::array<etl::span<float, BlockSize>, 2> output;
    };

    Hades() = default;

    auto prepare(float sampleRate, etl::size_t blockSize) -> void
    {
        auto const blockRate = sampleRate / static_cast<float>(blockSize);

        _textureKnob.prepare(blockRate);
        _morphKnob.prepare(blockRate);
        _ampKnob.prepare(blockRate);
        _compressorKnob.prepare(blockRate);
        _morphCV.prepare(blockRate);
        _sideChainCV.prepare(blockRate);
        _attackCV.prepare(blockRate);
        _releaseCV.prepare(blockRate);

        _channels[0].prepare(sampleRate);
        _channels[1].prepare(sampleRate);
    }

    template<size_t BlockSize>
    [[nodiscard]] auto processBlock(Buffers<BlockSize> const& context, ControlInputs const& inputs) -> ControlOutputs
    {
        auto const textureKnob    = _textureKnob.process(inputs.textureKnob);
        auto const morphKnob      = _morphKnob.process(inputs.morphKnob);
        auto const ampKnob        = _ampKnob.process(inputs.ampKnob);
        auto const compressorKnob = _compressorKnob.process(inputs.compressorKnob);
        auto const morphCV        = _morphCV.process(inputs.morphCV);
        auto const sideChainCV    = _sideChainCV.process(inputs.sideChainCV);
        auto const attackCV       = _attackCV.process(inputs.attackCV);
        auto const releaseCV      = _releaseCV.process(inputs.releaseCV);

        auto const channelParameter = Channel::Parameter{
            .texture    = textureKnob,
            .morph      = etl::clamp(morphKnob + morphCV, 0.0F, 1.0F),
            .amp        = ampKnob,
            .compressor = compressorKnob,
            .sideChain  = sideChainCV,
            .attack     = attackCV,
            .release    = releaseCV,
        };

        for (auto& channel : _channels) { channel.setParameter(channelParameter); }

        for (size_t i = 0; i < BlockSize; ++i)
        {
            auto const left  = context.input[0][i];
            auto const right = context.input[1][i];

            context.output[0][i] = _channels[0].processSample(left);
            context.output[1][i] = _channels[1].processSample(right);
        }

        //  "DIGITAL" GATE LOGIC
        auto const gateOut = inputs.gate1 != inputs.gate2;

        return {
            .envelope = 0.0F,
            .gate1    = gateOut,
            .gate2    = not gateOut,
        };
    }

private:
    mc::DynamicSmoothing<float> _textureKnob;
    mc::DynamicSmoothing<float> _morphKnob;
    mc::DynamicSmoothing<float> _ampKnob;
    mc::DynamicSmoothing<float> _compressorKnob;
    mc::DynamicSmoothing<float> _morphCV;
    mc::DynamicSmoothing<float> _sideChainCV;
    mc::DynamicSmoothing<float> _attackCV;
    mc::DynamicSmoothing<float> _releaseCV;

    etl::array<Channel, 2> _channels{};
};

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto patch = daisy::patch_sm::DaisyPatchSM{};
auto hades = Hades{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    auto const leftIn   = etl::span<float const, BLOCK_SIZE>{etl::addressof(IN_L[0]), BLOCK_SIZE};
    auto const rightIn  = etl::span<float const, BLOCK_SIZE>{etl::addressof(IN_R[0]), BLOCK_SIZE};
    auto const leftOut  = etl::span<float, BLOCK_SIZE>{etl::addressof(OUT_L[0]), BLOCK_SIZE};
    auto const rightOut = etl::span<float, BLOCK_SIZE>{etl::addressof(OUT_R[0]), BLOCK_SIZE};
    auto const context  = Hades::Buffers<BLOCK_SIZE>{
         .input  = {leftIn, rightIn},
         .output = {leftOut, rightOut},
    };

    auto const inputs = Hades::ControlInputs{
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
