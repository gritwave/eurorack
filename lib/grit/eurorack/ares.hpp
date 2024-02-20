#pragma once

#include <grit/audio/airwindows/airwindows_fire_amp.hpp>
#include <grit/audio/airwindows/airwindows_grind_amp.hpp>
#include <grit/audio/filter/dynamic_smoothing.hpp>
#include <grit/audio/stereo/stereo_block.hpp>

#include <etl/array.hpp>

namespace grit {

/// \ingroup grit-eurorack
struct Ares
{
    enum struct Mode
    {
        Fire,
        Grind,
    };

    struct ControlInput
    {
        Mode mode{Mode::Fire};

        float gainKnob{0};
        float toneKnob{0};
        float outputKnob{0};
        float mixKnob{0};

        float gainCV{0};
        float toneCV{0};
        float outputCV{0};
        float mixCV{0};
    };

    Ares() = default;

    auto prepare(float sampleRate, etl::size_t blockSize) -> void;
    auto process(StereoBlock<float> const& buffer, ControlInput const& inputs) -> void;

private:
    struct Channel
    {
        struct Parameter
        {
            Mode mode{Mode::Fire};
            float gain{0};
            float tone{0};
            float output{0};
            float mix{0};
        };

        Channel() = default;

        auto setParameter(Parameter const& parameter) -> void;
        auto setSampleRate(float sampleRate) -> void;

        [[nodiscard]] auto operator()(float sample) -> float;

    private:
        Mode _mode{Mode::Fire};

        AirWindowsFireAmp<float> _fire{};
        AirWindowsGrindAmp<float> _grind{};
    };

    DynamicSmoothing<float> _gainKnob{};
    DynamicSmoothing<float> _toneKnob{};
    DynamicSmoothing<float> _outputKnob{};
    DynamicSmoothing<float> _mixKnob{};
    DynamicSmoothing<float> _gainCV{};
    DynamicSmoothing<float> _toneCV{};
    DynamicSmoothing<float> _outputCV{};
    DynamicSmoothing<float> _mixCV{};

    etl::array<Channel, 2> _channels{};
};

}  // namespace grit
