#pragma once

#include <grit/audio/dynamic/compressor.hpp>
#include <grit/audio/envelope/envelope_follower.hpp>
#include <grit/audio/noise/white_noise.hpp>
#include <grit/audio/unit/decibel.hpp>
#include <grit/audio/waveshape/wave_shaper.hpp>
#include <grit/math/dynamic_smoothing.hpp>

#include <etl/array.hpp>
#include <etl/functional.hpp>
#include <etl/span.hpp>

namespace grit {

struct hades
{
    struct control_inputs
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

    struct control_outputs
    {
        float envelope{0};
        bool gate1{false};
        bool gate2{false};
    };

    struct buffers
    {
        etl::array<etl::span<float const>, 2> input;
        etl::array<etl::span<float>, 2> output;
    };

    hades() = default;

    auto prepare(float sample_rate, etl::size_t block_size) -> void;
    [[nodiscard]] auto process_block(buffers const& context, control_inputs const& inputs) -> control_outputs;

private:
    struct channel
    {
        struct parameter
        {
            float texture{0.0F};
            float morph{0.0F};
            float amp{0.0F};
            float compressor{0.0F};
            float sideChain{0.0F};
            float attack{0.0F};
            float release{0.0F};
        };

        channel() = default;

        auto set_parameter(parameter const& parameter) -> void;

        auto prepare(float sample_rate) -> void;
        [[nodiscard]] auto process_sample(float sample) -> float;

    private:
        parameter _parameter{};
        envelope_follower<float> _envelope_follower{};
        white_noise<float> _noise{};
        wave_shaper<float> _wave_shaper{etl::tanh};
        compressor<float> _compressor;
    };

    dynamic_smoothing<float> _texture_knob;
    dynamic_smoothing<float> _morph_knob;
    dynamic_smoothing<float> _amp_knob;
    dynamic_smoothing<float> _compressor_knob;
    dynamic_smoothing<float> _morph_cv;
    dynamic_smoothing<float> _side_chain_cv;
    dynamic_smoothing<float> _attack_cv;
    dynamic_smoothing<float> _release_cv;

    etl::array<channel, 2> _channels{};
};

inline auto hades::channel::set_parameter(parameter const& parameter) -> void { _parameter = parameter; }

inline auto hades::channel::prepare(float sample_rate) -> void
{
    _envelope_follower.prepare(sample_rate);
    _compressor.prepare(sample_rate);
}

inline auto hades::channel::process_sample(float sample) -> float
{
    _envelope_follower.set_parameter({
        .attack  = milliseconds<float>{50},
        .release = milliseconds<float>{50},
    });

    _compressor.set_parameter({
        .threshold = from_decibels(-12.0F),
        .ratio     = 10.F,
        .knee      = 1.0F,
        .attack    = milliseconds<float>{50},
        .release   = milliseconds<float>{50},
        .makeUp    = 1.0F,
        .wet       = 1.0F,
    });

    auto const env      = _envelope_follower.process_sample(sample);
    auto const noise    = _noise.process_sample();
    auto const noisy    = sample + noise * env;
    auto const dist_out = _wave_shaper.process_sample(noisy);
    return _compressor.process_sample(dist_out, dist_out);
}

inline auto hades::prepare(float sample_rate, etl::size_t block_size) -> void
{
    auto const block_rate = sample_rate / static_cast<float>(block_size);

    _texture_knob.prepare(block_rate);
    _morph_knob.prepare(block_rate);
    _amp_knob.prepare(block_rate);
    _compressor_knob.prepare(block_rate);
    _morph_cv.prepare(block_rate);
    _side_chain_cv.prepare(block_rate);
    _attack_cv.prepare(block_rate);
    _release_cv.prepare(block_rate);

    _channels[0].prepare(sample_rate);
    _channels[1].prepare(sample_rate);
}

inline auto hades::process_block(buffers const& context, control_inputs const& inputs) -> control_outputs
{
    auto const texture_knob    = _texture_knob.process(inputs.textureKnob);
    auto const morph_knob      = _morph_knob.process(inputs.morphKnob);
    auto const amp_knob        = _amp_knob.process(inputs.ampKnob);
    auto const compressor_knob = _compressor_knob.process(inputs.compressorKnob);
    auto const morph_cv        = _morph_cv.process(inputs.morphCV);
    auto const side_chain_cv   = _side_chain_cv.process(inputs.sideChainCV);
    auto const attack_cv       = _attack_cv.process(inputs.attackCV);
    auto const release_cv      = _release_cv.process(inputs.releaseCV);

    auto const channel_parameter = hades::channel::parameter{
        .texture    = texture_knob,
        .morph      = etl::clamp(morph_knob + morph_cv, 0.0F, 1.0F),
        .amp        = amp_knob,
        .compressor = compressor_knob,
        .sideChain  = side_chain_cv,
        .attack     = attack_cv,
        .release    = release_cv,
    };

    for (auto& channel : _channels) {
        channel.set_parameter(channel_parameter);
    }

    for (size_t i = 0; i < context.input[0].size(); ++i) {
        auto const left  = context.input[0][i];
        auto const right = context.input[1][i];

        context.output[0][i] = _channels[0].process_sample(left);
        context.output[1][i] = _channels[1].process_sample(right);
    }

    // "DIGITAL" GATE LOGIC
    auto const gate_out = inputs.gate1 != inputs.gate2;

    return {
        .envelope = 0.0F,
        .gate1    = gate_out,
        .gate2    = not gate_out,
    };
}

}  // namespace grit
