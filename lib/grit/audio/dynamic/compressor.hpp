#pragma once

#include <grit/audio/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/optional.hpp>

#include <cmath>

namespace grit {

template<etl::floating_point Float>
struct compressor
{
    struct parameter
    {
        Float threshold{0};
        Float ratio{1};
        Float knee{0};

        milliseconds<Float> attack{0};
        milliseconds<Float> release{0};

        Float makeUp{0};
        Float wet{0};
    };

    compressor() = default;

    auto set_parameter(parameter const& parameter) noexcept -> void;

    auto reset() noexcept -> void;
    auto prepare(Float sampleRate) noexcept -> void;
    [[nodiscard]] auto process_sample(Float signal, Float sideChain) noexcept -> Float;

    [[nodiscard]] auto get_gain_reduction() const noexcept -> Float;

private:
    [[nodiscard]] auto calculate_time_alpha(seconds<Float> value) const noexcept -> Float;

    parameter _parameter{};
    Float _sample_rate{};
    Float _yl_prev{};
    Float _reduction{1};
};

template<etl::floating_point Float>
auto compressor<Float>::set_parameter(parameter const& parameter) noexcept -> void
{
    _parameter = parameter;
}

template<etl::floating_point Float>
auto compressor<Float>::prepare(Float sampleRate) noexcept -> void
{
    _sample_rate = sampleRate;
    reset();
}

template<etl::floating_point Float>
auto compressor<Float>::process_sample(Float signal, Float sideChain) noexcept -> Float
{
    auto const threshold = _parameter.threshold;
    auto const ratio     = _parameter.ratio;
    auto const knee      = _parameter.knee;
    auto const alpha_a   = calculate_time_alpha(_parameter.attack);
    auto const alpha_r   = calculate_time_alpha(_parameter.release);

    auto const in  = sideChain * sideChain;
    auto const env = in <= static_cast<Float>(1e-6) ? -Float(60) : Float(10) * std::log10(in);

    auto const half_knee_range = -(knee * (-Float(60) - threshold) / Float(4));
    auto const full_knee_range = half_knee_range + half_knee_range / ratio;
    auto const kneed_threshold = threshold - half_knee_range;
    auto const ceil_threshold  = threshold + half_knee_range / ratio;
    auto const limit           = etl::clamp(env, kneed_threshold, ceil_threshold);
    auto const factor          = -((limit - kneed_threshold) / full_knee_range) + Float(1);
    auto const ratio_quotient  = knee > 0 ? ratio * factor + Float(1) * (-factor + Float(1)) : Float(1);

    auto yg = Float(0);
    if (env < kneed_threshold) {
        yg = env;
    } else {
        yg = kneed_threshold + (env - kneed_threshold) / (ratio / ratio_quotient);
    }

    auto yl       = Float(0);
    auto const xl = env - yg;
    if (xl > _yl_prev) {
        yl = alpha_a * _yl_prev + (Float(1) - alpha_a) * xl;
    } else {
        yl = alpha_r * _yl_prev + (Float(1) - alpha_r) * xl;
    }

    auto const control_compressor = etl::pow(Float(10), (Float(1) - yl) * Float(0.05));

    _reduction = control_compressor;
    _yl_prev   = yl;

    auto const wet    = _parameter.wet;
    auto const makeup = _parameter.makeUp;

    auto const wet_sample = signal * control_compressor * makeup;
    auto const dry_sample = signal;

    return wet_sample * wet + dry_sample * (1.0F - wet);
}

template<etl::floating_point Float>
auto compressor<Float>::get_gain_reduction() const noexcept -> Float
{
    return _reduction;
}

template<etl::floating_point Float>
auto compressor<Float>::reset() noexcept -> void
{
    _yl_prev = Float(0);
}

template<etl::floating_point Float>
auto compressor<Float>::calculate_time_alpha(seconds<Float> value) const noexcept -> Float
{
    static constexpr auto const euler = static_cast<Float>(etl::numbers::e);

    auto const sec = value.count();
    if (sec == Float(0)) {
        return Float(0);
    }
    return etl::pow(Float(1) / euler, Float(1) / static_cast<Float>(_sample_rate) / sec);
}

}  // namespace grit
