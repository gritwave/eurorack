#pragma once

#include <grit/audio/airwindows/airwindows_fire_amp.hpp>
#include <grit/audio/airwindows/airwindows_grind_amp.hpp>
#include <grit/audio/airwindows/airwindows_vinyl_dither.hpp>
#include <grit/audio/dynamic/compressor.hpp>
#include <grit/audio/envelope/envelope_follower.hpp>
#include <grit/audio/filter/dynamic_smoothing.hpp>
#include <grit/audio/mix/cross_fade.hpp>
#include <grit/audio/noise/white_noise.hpp>
#include <grit/audio/stereo/stereo_block.hpp>
#include <grit/audio/waveshape/diode_rectifier.hpp>
#include <grit/audio/waveshape/full_wave_rectifier.hpp>
#include <grit/audio/waveshape/half_wave_rectifier.hpp>
#include <grit/audio/waveshape/hard_clipper.hpp>
#include <grit/audio/waveshape/tanh_clipper.hpp>
#include <grit/math/normalizable_range.hpp>
#include <grit/math/remap.hpp>
#include <grit/unit/decibel.hpp>

#include <etl/algorithm.hpp>
#include <etl/array.hpp>
#include <etl/functional.hpp>
#include <etl/utility.hpp>
#include <etl/variant.hpp>

namespace grit {

/// \ingroup grit-eurorack
struct Hades
{
    struct ControlInput
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

    struct ControlOutput
    {
        float envelope{0};
        bool gate1{false};
        bool gate2{false};
    };

    Hades() = default;

    auto nextTextureAlgorithm() -> void;
    auto nextDistortionAlgorithm() -> void;

    auto prepare(float sampleRate, etl::size_t blockSize) -> void;
    [[nodiscard]] auto process(StereoBlock<float> const& buffer, ControlInput const& inputs) -> ControlOutput;

private:
    struct Amp
    {
        Amp() = default;

        auto next() -> void;
        auto setSampleRate(float sampleRate) -> void;
        [[nodiscard]] auto operator()(float sample) -> float;

    private:
        enum Index : int
        {
            TanhIndex = 0,
            HardIndex,
            FullWaveIndex,
            HalfWaveIndex,
            DiodeIndex,
            FireAmpIndex,
            GrindAmpIndex,
            MaxIndex,
        };

        Index _index{TanhIndex};
        TanhClipperADAA1<float> _tanh{};
        HardClipper<float> _hard{};
        FullWaveRectifier<float> _fullWave{};
        HalfWaveRectifier<float> _halfWave{};
        DiodeRectifier<float> _diode{};
        AirWindowsFireAmp<float> _fireAmp{42};
        AirWindowsGrindAmp<float> _grindAmp{143};
    };

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

        auto setParameter(Parameter const& parameter) -> void;
        auto nextDistortionAlgorithm() -> void;

        auto setSampleRate(float sampleRate) -> void;
        [[nodiscard]] auto operator()(float sample) -> etl::pair<float, float>;

    private:
        static constexpr auto attackRange  = NormalizableRange<float>{1.0F, 100.0F, 25.0F};
        static constexpr auto releaseRange = NormalizableRange<float>{1.0F, 500.0F, 100.0F};

        Parameter _parameter{};

        EnvelopeFollower<float> _envelope{};
        WhiteNoise<float> _whiteNoise{};
        AirWindowsVinylDither<float> _vinyl{};
        Amp _distortion{};
        SoftKneeCompressor<float> _compressor{};
    };

    DynamicSmoothing<float> _textureKnob{};
    DynamicSmoothing<float> _morphKnob{};
    DynamicSmoothing<float> _ampKnob{};
    DynamicSmoothing<float> _compressorKnob{};
    DynamicSmoothing<float> _morphCv{};
    DynamicSmoothing<float> _sideChainCv{};
    DynamicSmoothing<float> _attackCv{};
    DynamicSmoothing<float> _releaseCv{};

    etl::array<Channel, 2> _channels{};
};

}  // namespace grit
