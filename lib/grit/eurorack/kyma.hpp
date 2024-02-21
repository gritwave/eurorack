#pragma once

#include <grit/audio/envelope/envelope_adsr.hpp>
#include <grit/audio/filter/dynamic_smoothing.hpp>
#include <grit/audio/oscillator/variable_shape_oscillator.hpp>
#include <grit/audio/oscillator/wavetable_oscillator.hpp>
#include <grit/audio/stereo/stereo_block.hpp>

namespace grit {

/// \ingroup grit-eurorack
struct Kyma
{
    struct ControlInput
    {
        float pitchKnob{0};
        float morphKnob{0};
        float attackKnob{0};
        float releaseKnob{0};

        float vOctCV{0};
        float morphCV{0};
        float subGainCV{0};
        float subMorphCV{0};

        bool gate{false};
        bool subShift{false};
    };

    Kyma() = default;

    auto prepare(float sampleRate, etl::size_t blockSize) -> void;
    auto process(StereoBlock<float> const& buffer, ControlInput const& inputs) -> float;

private:
    static constexpr auto sine      = makeSineWavetable<float, 2048>();
    static constexpr auto wavetable = etl::mdspan{sine.data(), etl::extents<etl::size_t, sine.size()>{}};

    float _sampleRate{};

    DynamicSmoothing<float> _pitchKnob{};
    DynamicSmoothing<float> _morphKnob{};
    DynamicSmoothing<float> _attackKnob{};
    DynamicSmoothing<float> _releaseKnob{};
    DynamicSmoothing<float> _vOctCV{};
    DynamicSmoothing<float> _morphCV{};
    DynamicSmoothing<float> _subGainCV{};
    DynamicSmoothing<float> _subMorphCV{};

    EnvelopeADSR<float> _adsr{};
    WavetableOscillator<float, sine.size()> _oscillator{wavetable};
    WavetableOscillator<float, sine.size()> _subOscillator{wavetable};
};

}  // namespace grit
