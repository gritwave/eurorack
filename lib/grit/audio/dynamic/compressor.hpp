#pragma once

#include <grit/audio/unit/time.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/numbers.hpp>
#include <etl/optional.hpp>

#include <cmath>

namespace grit {

template<etl::floating_point SampleType>
struct compressor
{
    struct parameter
    {
        SampleType threshold{0};
        SampleType ratio{1};
        SampleType knee{0};

        milliseconds<SampleType> attack{0};
        milliseconds<SampleType> release{0};

        SampleType makeUp{0};
        SampleType wet{0};
    };

    compressor() = default;

    auto set_parameter(parameter const& parameter) noexcept -> void;

    auto reset() noexcept -> void;
    auto prepare(SampleType sampleRate) noexcept -> void;
    [[nodiscard]] auto process_sample(SampleType signal, SampleType sideChain) noexcept -> SampleType;

    [[nodiscard]] auto get_gain_reduction() const noexcept -> SampleType;

private:
    [[nodiscard]] auto calculate_time_alpha(seconds<SampleType> value) const noexcept -> SampleType;

    parameter _parameter{};
    SampleType _sample_rate{};
    SampleType _yl_prev{};
    SampleType _reduction{1};
};

template<etl::floating_point SampleType>
auto compressor<SampleType>::set_parameter(parameter const& parameter) noexcept -> void
{
    _parameter = parameter;
}

template<etl::floating_point SampleType>
auto compressor<SampleType>::prepare(SampleType sampleRate) noexcept -> void
{
    _sample_rate = sampleRate;
    reset();
}

template<etl::floating_point SampleType>
auto compressor<SampleType>::process_sample(SampleType signal, SampleType sideChain) noexcept -> SampleType
{
    auto const threshold = _parameter.threshold;
    auto const ratio     = _parameter.ratio;
    auto const knee      = _parameter.knee;
    auto const alpha_a   = calculate_time_alpha(_parameter.attack);
    auto const alpha_r   = calculate_time_alpha(_parameter.release);

    auto const in  = sideChain * sideChain;
    auto const env = in <= static_cast<SampleType>(1e-6) ? -SampleType(60) : SampleType(10) * std::log10(in);

    auto const half_knee_range = -(knee * (-SampleType(60) - threshold) / SampleType(4));
    auto const full_knee_range = half_knee_range + half_knee_range / ratio;
    auto const kneed_threshold = threshold - half_knee_range;
    auto const ceil_threshold  = threshold + half_knee_range / ratio;
    auto const limit           = etl::clamp(env, kneed_threshold, ceil_threshold);
    auto const factor          = -((limit - kneed_threshold) / full_knee_range) + SampleType(1);
    auto const ratio_quotient  = knee > 0 ? ratio * factor + SampleType(1) * (-factor + SampleType(1)) : SampleType(1);

    auto yg = SampleType(0);
    if (env < kneed_threshold) {
        yg = env;
    } else {
        yg = kneed_threshold + (env - kneed_threshold) / (ratio / ratio_quotient);
    }

    auto yl       = SampleType(0);
    auto const xl = env - yg;
    if (xl > _yl_prev) {
        yl = alpha_a * _yl_prev + (SampleType(1) - alpha_a) * xl;
    } else {
        yl = alpha_r * _yl_prev + (SampleType(1) - alpha_r) * xl;
    }

    auto const control_compressor = etl::pow(SampleType(10), (SampleType(1) - yl) * SampleType(0.05));

    _reduction = control_compressor;
    _yl_prev   = yl;

    auto const wet    = _parameter.wet;
    auto const makeup = _parameter.makeUp;

    auto const wet_sample = signal * control_compressor * makeup;
    auto const dry_sample = signal;

    return wet_sample * wet + dry_sample * (1.0F - wet);
}

template<etl::floating_point SampleType>
auto compressor<SampleType>::get_gain_reduction() const noexcept -> SampleType
{
    return _reduction;
}

template<etl::floating_point SampleType>
auto compressor<SampleType>::reset() noexcept -> void
{
    _yl_prev = SampleType(0);
}

template<etl::floating_point SampleType>
auto compressor<SampleType>::calculate_time_alpha(seconds<SampleType> value) const noexcept -> SampleType
{
    static constexpr auto const euler = static_cast<SampleType>(etl::numbers::e);

    auto const sec = value.count();
    if (sec == SampleType(0)) {
        return SampleType(0);
    }
    return etl::pow(SampleType(1) / euler, SampleType(1) / static_cast<SampleType>(_sample_rate) / sec);
}

}  // namespace grit
