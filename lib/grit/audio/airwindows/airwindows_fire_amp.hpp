#pragma once

#include <grit/math/static_lookup_table_transform.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace grit {

/// \ingroup grit-audio-airwindows
template<etl::floating_point Float, typename URNG = etl::xoshiro128plusplus>
struct AirWindowsFireAmp
{
    using value_type = Float;
    using seed_type  = typename URNG::result_type;

    struct Parameter
    {
        Float gain{0.5};
        Float tone{0.5};
        Float output{0.8};
        Float mix{1};
    };

    AirWindowsFireAmp() = default;
    explicit AirWindowsFireAmp(seed_type seed);

    auto setParameter(Parameter parameter) -> void;
    auto setSampleRate(Float sampleRate) -> void;

    [[nodiscard]] auto operator()(Float x) -> Float;
    auto reset() -> void;

private:
    static constexpr auto sineLUT = StaticLookupTableTransform<Float, 255>{
        [](auto x) { return etl::sin(x); },
        Float(0),
        Float(1.57079633),
    };

    URNG _rng{42};
    etl::uniform_real_distribution<Float> _dist{Float(0), Float(1)};

    Parameter _parameter{};
    Float _sampleRate{};

    Float _lastSampleL{0};
    Float _storeSampleL{0};
    Float _smoothAl{0};
    Float _smoothBl{0};
    Float _smoothCl{0};
    Float _smoothDl{0};
    Float _smoothEl{0};
    Float _smoothFl{0};
    Float _smoothGl{0};
    Float _smoothHl{0};
    Float _smoothIl{0};
    Float _smoothJl{0};
    Float _smoothKl{0};
    Float _smoothLl{0};
    Float _iirSampleAl{0};
    Float _iirSampleBl{0};
    Float _iirSampleCl{0};
    Float _iirSampleDl{0};
    Float _iirSampleEl{0};
    Float _iirSampleFl{0};
    Float _iirSampleGl{0};
    Float _iirSampleHl{0};
    Float _iirSampleIl{0};
    Float _iirSampleJl{0};
    Float _iirSampleKl{0};
    Float _iirSampleLl{0};
    Float _iirLowpassL{0};
    Float _iirSpkAl{0};
    Float _iirSpkBl{0};
    Float _iirSubL{0};
    Float _oddL[257]{0};
    Float _evenL[257]{0};

    bool _flip{false};
    int _count{0};  // amp

    Float _bL[90]{0};
    Float _lastCabSampleL{0};
    Float _smoothCabAl{0};
    Float _smoothCabBl{0};  // cab

    Float _lastRefL[10]{0};
    int _cycle{0};  // undersampling

    enum
    {
        FixFreq,
        FixReso,
        FixA0,
        FixA1,
        FixA2,
        FixB1,
        FixB2,
        FixSL1,
        FixSL2,
        FixTotal
    };

    Float _fixA[FixTotal]{0};
    Float _fixB[FixTotal]{0};
    Float _fixC[FixTotal]{0};
    Float _fixD[FixTotal]{0};
    Float _fixE[FixTotal]{0};
    Float _fixF[FixTotal]{0};  // filtering

    Float _startlevel{0};
    Float _bassfill{0};
    Float _outputlevel{0};
    Float _toneEq{0};
    Float _eq{0};
    Float _bleed{0};
    Float _bassfactor{0};
    Float _beq{0};
    Float _wet{0};
    int _cycleEnd{0};
    int _diagonal{0};
    int _down{0};
    int _side{0};
};

template<etl::floating_point Float, typename URNG>
AirWindowsFireAmp<Float, URNG>::AirWindowsFireAmp(seed_type seed) : _rng{seed}
{
    reset();
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsFireAmp<Float, URNG>::setParameter(Parameter parameter) -> void
{
    _parameter = parameter;

    static constexpr auto const pi = static_cast<Float>(etl::numbers::pi);

    auto const a = _parameter.gain;
    auto const b = _parameter.tone;
    auto const c = _parameter.output;
    auto const d = _parameter.mix;

    _bassfill    = a;
    _outputlevel = c;
    _wet         = d;

    auto overallscale = Float(1);
    overallscale /= 44100.0;
    overallscale *= _sampleRate;

    _cycleEnd = etl::floor(overallscale);
    if (_cycleEnd < 1) {
        _cycleEnd = 1;
    }
    if (_cycleEnd > 4) {
        _cycleEnd = 4;
    }
    // this is going to be 2 for 88.1 or 96k, 3 for silly people, 4 for 176 or 192k
    if (_cycle > _cycleEnd - 1) {
        _cycle = _cycleEnd - 1;  // sanity check
    }

    _startlevel      = _bassfill;
    Float samplerate = _sampleRate;
    Float basstrim   = _bassfill / 16.0;
    _toneEq          = (b / samplerate) * 22050.0;
    _eq              = (basstrim / samplerate) * 22050.0;
    _bleed           = _outputlevel / 16.0;
    _bassfactor      = Float(1) - (basstrim * basstrim);
    _beq             = (_bleed / samplerate) * 22050.0;

    _diagonal = (int)(0.000861678 * samplerate);
    if (_diagonal > 127) {
        _diagonal = 127;
    }
    _side = (int)(_diagonal / 1.4142135623730951);
    _down = (_side + _diagonal) / 2;
    // now we've got down, side and diagonal as offsets and we also use three successive samples upfront

    Float cutoff = (15000.0 + (b * 10000.0)) / _sampleRate;
    if (cutoff > 0.49) {
        cutoff = 0.49;  // don't crash if run at 44.1k
    }
    if (cutoff < 0.001) {
        cutoff = 0.001;  // or if cutoff's too low
    }

    _fixF[FixFreq] = _fixE[FixFreq] = _fixD[FixFreq] = _fixC[FixFreq] = _fixB[FixFreq] = _fixA[FixFreq] = cutoff;

    _fixA[FixReso] = Float(4.46570214);
    _fixB[FixReso] = Float(1.51387132);
    _fixC[FixReso] = Float(0.93979296);
    _fixD[FixReso] = Float(0.70710678);
    _fixE[FixReso] = Float(0.52972649);
    _fixF[FixReso] = Float(0.50316379);

    Float k      = etl::tan(pi * _fixA[FixFreq]);  // lowpass
    Float norm   = Float(1) / (Float(1) + k / _fixA[FixReso] + k * k);
    _fixA[FixA0] = k * k * norm;
    _fixA[FixA1] = Float(2) * _fixA[FixA0];
    _fixA[FixA2] = _fixA[FixA0];
    _fixA[FixB1] = Float(2) * (k * k - Float(1)) * norm;
    _fixA[FixB2] = (Float(1) - k / _fixA[FixReso] + k * k) * norm;

    k            = etl::tan(pi * _fixB[FixFreq]);
    norm         = Float(1) / (Float(1) + k / _fixB[FixReso] + k * k);
    _fixB[FixA0] = k * k * norm;
    _fixB[FixA1] = Float(2) * _fixB[FixA0];
    _fixB[FixA2] = _fixB[FixA0];
    _fixB[FixB1] = Float(2) * (k * k - Float(1)) * norm;
    _fixB[FixB2] = (Float(1) - k / _fixB[FixReso] + k * k) * norm;

    k            = etl::tan(pi * _fixC[FixFreq]);
    norm         = Float(1) / (Float(1) + k / _fixC[FixReso] + k * k);
    _fixC[FixA0] = k * k * norm;
    _fixC[FixA1] = Float(2) * _fixC[FixA0];
    _fixC[FixA2] = _fixC[FixA0];
    _fixC[FixB1] = Float(2) * (k * k - Float(1)) * norm;
    _fixC[FixB2] = (Float(1) - k / _fixC[FixReso] + k * k) * norm;

    k            = etl::tan(pi * _fixD[FixFreq]);
    norm         = Float(1) / (Float(1) + k / _fixD[FixReso] + k * k);
    _fixD[FixA0] = k * k * norm;
    _fixD[FixA1] = Float(2) * _fixD[FixA0];
    _fixD[FixA2] = _fixD[FixA0];
    _fixD[FixB1] = Float(2) * (k * k - Float(1)) * norm;
    _fixD[FixB2] = (Float(1) - k / _fixD[FixReso] + k * k) * norm;

    k            = etl::tan(pi * _fixE[FixFreq]);
    norm         = Float(1) / (Float(1) + k / _fixE[FixReso] + k * k);
    _fixE[FixA0] = k * k * norm;
    _fixE[FixA1] = Float(2) * _fixE[FixA0];
    _fixE[FixA2] = _fixE[FixA0];
    _fixE[FixB1] = Float(2) * (k * k - Float(1)) * norm;
    _fixE[FixB2] = (Float(1) - k / _fixE[FixReso] + k * k) * norm;

    k            = etl::tan(pi * _fixF[FixFreq]);
    norm         = Float(1) / (Float(1) + k / _fixF[FixReso] + k * k);
    _fixF[FixA0] = k * k * norm;
    _fixF[FixA1] = Float(2) * _fixF[FixA0];
    _fixF[FixA2] = _fixF[FixA0];
    _fixF[FixB1] = Float(2) * (k * k - Float(1)) * norm;
    _fixF[FixB2] = (Float(1) - k / _fixF[FixReso] + k * k) * norm;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsFireAmp<Float, URNG>::setSampleRate(Float sampleRate) -> void
{
    _sampleRate = sampleRate;
    reset();
    setParameter(_parameter);
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsFireAmp<Float, URNG>::operator()(Float const x) -> Float
{
    auto inputSampleL = x;
    auto drySampleL   = inputSampleL;

    auto outSample = (inputSampleL * _fixA[FixA0]) + _fixA[FixSL1];
    _fixA[FixSL1]  = (inputSampleL * _fixA[FixA1]) - (outSample * _fixA[FixB1]) + _fixA[FixSL2];
    _fixA[FixSL2]  = (inputSampleL * _fixA[FixA2]) - (outSample * _fixA[FixB2]);
    inputSampleL   = outSample;  // fixed biquad filtering ultrasonics

    if (inputSampleL > Float(1)) {
        inputSampleL = Float(1);
    }
    if (inputSampleL < -Float(1)) {
        inputSampleL = -Float(1);
    }
    auto basscutL = Float(0.98);
    // we're going to be shifting this as the stages progress
    auto inputlevelL = _startlevel;
    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleAl = (_iirSampleAl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleAl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    auto bridgerectifier = (_smoothAl + inputSampleL);
    _smoothAl            = inputSampleL;
    inputSampleL         = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleBl = (_iirSampleBl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleBl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothBl + inputSampleL);
    _smoothBl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * _fixB[FixA0]) + _fixB[FixSL1];
    _fixB[FixSL1] = (inputSampleL * _fixB[FixA1]) - (outSample * _fixB[FixB1]) + _fixB[FixSL2];
    _fixB[FixSL2] = (inputSampleL * _fixB[FixA2]) - (outSample * _fixB[FixB2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleCl = (_iirSampleCl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleCl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothCl + inputSampleL);
    _smoothCl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleDl = (_iirSampleDl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleDl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothDl + inputSampleL);
    _smoothDl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * _fixC[FixA0]) + _fixC[FixSL1];
    _fixC[FixSL1] = (inputSampleL * _fixC[FixA1]) - (outSample * _fixC[FixB1]) + _fixC[FixSL2];
    _fixC[FixSL2] = (inputSampleL * _fixC[FixA2]) - (outSample * _fixC[FixB2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleEl = (_iirSampleEl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleEl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothEl + inputSampleL);
    _smoothEl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleFl = (_iirSampleFl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleFl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothFl + inputSampleL);
    _smoothFl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * _fixD[FixA0]) + _fixD[FixSL1];
    _fixD[FixSL1] = (inputSampleL * _fixD[FixA1]) - (outSample * _fixD[FixB1]) + _fixD[FixSL2];
    _fixD[FixSL2] = (inputSampleL * _fixD[FixA2]) - (outSample * _fixD[FixB2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleGl = (_iirSampleGl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleGl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothGl + inputSampleL);
    _smoothGl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleHl = (_iirSampleHl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleHl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothHl + inputSampleL);
    _smoothHl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * _fixE[FixA0]) + _fixE[FixSL1];
    _fixE[FixSL1] = (inputSampleL * _fixE[FixA1]) - (outSample * _fixE[FixB1]) + _fixE[FixSL2];
    _fixE[FixSL2] = (inputSampleL * _fixE[FixA2]) - (outSample * _fixE[FixB2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleIl = (_iirSampleIl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleIl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothIl + inputSampleL);
    _smoothIl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleJl = (_iirSampleJl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleJl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothJl + inputSampleL);
    _smoothJl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * _fixF[FixA0]) + _fixF[FixSL1];
    _fixF[FixSL1] = (inputSampleL * _fixF[FixA1]) - (outSample * _fixF[FixB1]) + _fixF[FixSL2];
    _fixF[FixSL2] = (inputSampleL * _fixF[FixA2]) - (outSample * _fixF[FixB2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleKl = (_iirSampleKl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleKl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothKl + inputSampleL);
    _smoothKl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL  = ((inputlevelL * Float(7)) + Float(1)) * Float(0.125);
    _iirSampleLl = (_iirSampleLl * (Float(1) - _eq)) + (inputSampleL * _eq);
    basscutL *= _bassfactor;
    inputSampleL = inputSampleL - (_iirSampleLl * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (_smoothLl + inputSampleL);
    _smoothLl       = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    _iirLowpassL = (_iirLowpassL * (Float(1) - _toneEq)) + (inputSampleL * _toneEq);
    inputSampleL = _iirLowpassL;
    // lowpass. The only one of this type.
    // lowpass. The only one of this type.

    _iirSpkAl = (_iirSpkAl * (Float(1) - _beq)) + (inputSampleL * _beq);
    // extra lowpass for 4*12" speakers
    // extra lowpass for 4*12" speakers

    if (_count < 0 || _count > 128) {
        _count = 128;
    }
    auto resultBL = 0.0;
    if (_flip) {
        _oddL[_count + 128] = _oddL[_count] = _iirSpkAl;
        resultBL = (_oddL[_count + _down] + _oddL[_count + _side] + _oddL[_count + _diagonal]);
    } else {
        _evenL[_count + 128] = _evenL[_count] = _iirSpkAl;
        resultBL = (_evenL[_count + _down] + _evenL[_count + _side] + _evenL[_count + _diagonal]);
    }
    _count--;
    _iirSpkBl = (_iirSpkBl * (Float(1) - _beq)) + (resultBL * _beq);
    inputSampleL += (_iirSpkBl * _bleed);
    // extra lowpass for 4*12" speakers
    // extra lowpass for 4*12" speakers

    bridgerectifier = etl::abs(inputSampleL * _outputlevel);
    if (bridgerectifier > Float(1.57079633)) {
        bridgerectifier = Float(1.57079633);
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (inputSampleL > 0) {
        inputSampleL = bridgerectifier;
    } else {
        inputSampleL = -bridgerectifier;
    }

    if (bridgerectifier > Float(1.57079633)) {
        bridgerectifier = Float(1.57079633);
    }
    bridgerectifier = sineLUT(bridgerectifier);

    _iirSubL = (_iirSubL * (Float(1) - _beq)) + (inputSampleL * _beq);
    inputSampleL += (_iirSubL * _bassfill * _outputlevel);

    auto randy    = (_dist(_rng) * Float(0.053));
    inputSampleL  = ((inputSampleL * (Float(1) - randy)) + (_storeSampleL * randy)) * _outputlevel;
    _storeSampleL = inputSampleL;

    randy = (_dist(_rng) * Float(0.053));

    _flip = !_flip;

    if (_wet != Float(1)) {
        inputSampleL = (inputSampleL * _wet) + (drySampleL * (Float(1) - _wet));
    }
    // Dry/Wet control, defaults to the last slider
    // amp

    _cycle++;
    if (_cycle == _cycleEnd) {
        auto temp    = (inputSampleL + _smoothCabAl) * (Float(1) / Float(3));
        _smoothCabAl = inputSampleL;
        inputSampleL = temp;

        _bL[84] = _bL[83];
        _bL[83] = _bL[82];
        _bL[82] = _bL[81];
        _bL[81] = _bL[80];
        _bL[80] = _bL[79];
        _bL[79] = _bL[78];
        _bL[78] = _bL[77];
        _bL[77] = _bL[76];
        _bL[76] = _bL[75];
        _bL[75] = _bL[74];
        _bL[74] = _bL[73];
        _bL[73] = _bL[72];
        _bL[72] = _bL[71];
        _bL[71] = _bL[70];
        _bL[70] = _bL[69];
        _bL[69] = _bL[68];
        _bL[68] = _bL[67];
        _bL[67] = _bL[66];
        _bL[66] = _bL[65];
        _bL[65] = _bL[64];
        _bL[64] = _bL[63];
        _bL[63] = _bL[62];
        _bL[62] = _bL[61];
        _bL[61] = _bL[60];
        _bL[60] = _bL[59];
        _bL[59] = _bL[58];
        _bL[58] = _bL[57];
        _bL[57] = _bL[56];
        _bL[56] = _bL[55];
        _bL[55] = _bL[54];
        _bL[54] = _bL[53];
        _bL[53] = _bL[52];
        _bL[52] = _bL[51];
        _bL[51] = _bL[50];
        _bL[50] = _bL[49];
        _bL[49] = _bL[48];
        _bL[48] = _bL[47];
        _bL[47] = _bL[46];
        _bL[46] = _bL[45];
        _bL[45] = _bL[44];
        _bL[44] = _bL[43];
        _bL[43] = _bL[42];
        _bL[42] = _bL[41];
        _bL[41] = _bL[40];
        _bL[40] = _bL[39];
        _bL[39] = _bL[38];
        _bL[38] = _bL[37];
        _bL[37] = _bL[36];
        _bL[36] = _bL[35];
        _bL[35] = _bL[34];
        _bL[34] = _bL[33];
        _bL[33] = _bL[32];
        _bL[32] = _bL[31];
        _bL[31] = _bL[30];
        _bL[30] = _bL[29];
        _bL[29] = _bL[28];
        _bL[28] = _bL[27];
        _bL[27] = _bL[26];
        _bL[26] = _bL[25];
        _bL[25] = _bL[24];
        _bL[24] = _bL[23];
        _bL[23] = _bL[22];
        _bL[22] = _bL[21];
        _bL[21] = _bL[20];
        _bL[20] = _bL[19];
        _bL[19] = _bL[18];
        _bL[18] = _bL[17];
        _bL[17] = _bL[16];
        _bL[16] = _bL[15];
        _bL[15] = _bL[14];
        _bL[14] = _bL[13];
        _bL[13] = _bL[12];
        _bL[12] = _bL[11];
        _bL[11] = _bL[10];
        _bL[10] = _bL[9];
        _bL[9]  = _bL[8];
        _bL[8]  = _bL[7];
        _bL[7]  = _bL[6];
        _bL[6]  = _bL[5];
        _bL[5]  = _bL[4];
        _bL[4]  = _bL[3];
        _bL[3]  = _bL[2];
        _bL[2]  = _bL[1];
        _bL[1]  = _bL[0];
        _bL[0]  = inputSampleL;
        inputSampleL += (_bL[1] * (Float(1.31698250313308396) - (Float(0.08140616497621633) * etl::abs(_bL[1]))));
        inputSampleL += (_bL[2] * (Float(1.47229016949915326) - (Float(0.27680278993637253) * etl::abs(_bL[2]))));
        inputSampleL += (_bL[3] * (Float(1.30410109086044956) - (Float(0.35629113432046489) * etl::abs(_bL[3]))));
        inputSampleL += (_bL[4] * (Float(0.81766210474551260) - (Float(0.26808782337659753) * etl::abs(_bL[4]))));
        inputSampleL += (_bL[5] * (Float(0.19868872545506663) - (Float(0.11105517193919669) * etl::abs(_bL[5]))));
        inputSampleL -= (_bL[6] * (Float(0.39115909132567039) - (Float(0.12630622002682679) * etl::abs(_bL[6]))));
        inputSampleL -= (_bL[7] * (Float(0.76881891559343574) - (Float(0.40879849500403143) * etl::abs(_bL[7]))));
        inputSampleL -= (_bL[8] * (Float(0.87146861782680340) - (Float(0.59529560488000599) * etl::abs(_bL[8]))));
        inputSampleL -= (_bL[9] * (Float(0.79504575932563670) - (Float(0.60877047551611796) * etl::abs(_bL[9]))));
        inputSampleL -= (_bL[10] * (Float(0.61653017622406314) - (Float(0.47662851438557335) * etl::abs(_bL[10]))));
        inputSampleL -= (_bL[11] * (Float(0.40718195794382067) - (Float(0.24955839378539713) * etl::abs(_bL[11]))));
        inputSampleL -= (_bL[12] * (Float(0.31794900040616203) - (Float(0.04169792259600613) * etl::abs(_bL[12]))));
        inputSampleL -= (_bL[13] * (Float(0.41075032540217843) + (Float(0.00368483996076280) * etl::abs(_bL[13]))));
        inputSampleL -= (_bL[14] * (Float(0.56901352922170667) - (Float(0.11027360805893105) * etl::abs(_bL[14]))));
        inputSampleL -= (_bL[15] * (Float(0.62443222391889264) - (Float(0.22198075154245228) * etl::abs(_bL[15]))));
        inputSampleL -= (_bL[16] * (Float(0.53462856723129204) - (Float(0.22933544545324852) * etl::abs(_bL[16]))));
        inputSampleL -= (_bL[17] * (Float(0.34441703361995046) - (Float(0.12956809502269492) * etl::abs(_bL[17]))));
        inputSampleL -= (_bL[18] * (Float(0.13947052337867882) + (Float(0.00339775055962799) * etl::abs(_bL[18]))));
        inputSampleL += (_bL[19] * (Float(0.03771252648928484) - (Float(0.10863931549251718) * etl::abs(_bL[19]))));
        inputSampleL += (_bL[20] * (Float(0.18280210770271693) - (Float(0.17413646599296417) * etl::abs(_bL[20]))));
        inputSampleL += (_bL[21] * (Float(0.24621986701761467) - (Float(0.14547053270435095) * etl::abs(_bL[21]))));
        inputSampleL += (_bL[22] * (Float(0.22347075142737360) - (Float(0.02493869490104031) * etl::abs(_bL[22]))));
        inputSampleL += (_bL[23] * (Float(0.14346348482123716) + (Float(0.11284054747963246) * etl::abs(_bL[23]))));
        inputSampleL += (_bL[24] * (Float(0.00834364862916028) + (Float(0.24284684053733926) * etl::abs(_bL[24]))));
        inputSampleL -= (_bL[25] * (Float(0.11559740296078347) - (Float(0.32623054435304538) * etl::abs(_bL[25]))));
        inputSampleL -= (_bL[26] * (Float(0.18067604561283060) - (Float(0.32311481551122478) * etl::abs(_bL[26]))));
        inputSampleL -= (_bL[27] * (Float(0.22927997789035612) - (Float(0.26991539052832925) * etl::abs(_bL[27]))));
        inputSampleL -= (_bL[28] * (Float(0.28487666578669446) - (Float(0.22437227250279349) * etl::abs(_bL[28]))));
        inputSampleL -= (_bL[29] * (Float(0.31992973037153838) - (Float(0.15289876100963865) * etl::abs(_bL[29]))));
        inputSampleL -= (_bL[30] * (Float(0.35174606303520733) - (Float(0.05656293023086628) * etl::abs(_bL[30]))));
        inputSampleL -= (_bL[31] * (Float(0.36894898011375254) + (Float(0.04333925421463558) * etl::abs(_bL[31]))));
        inputSampleL -= (_bL[32] * (Float(0.32567576055307507) + (Float(0.14594589410921388) * etl::abs(_bL[32]))));
        inputSampleL -= (_bL[33] * (Float(0.27440135050585784) + (Float(0.15529667398122521) * etl::abs(_bL[33]))));
        inputSampleL -= (_bL[34] * (Float(0.21998973785078091) + (Float(0.05083553737157104) * etl::abs(_bL[34]))));
        inputSampleL -= (_bL[35] * (Float(0.10323624876862457) - (Float(0.04651829594199963) * etl::abs(_bL[35]))));
        inputSampleL += (_bL[36] * (Float(0.02091603687851074) + (Float(0.12000046818439322) * etl::abs(_bL[36]))));
        inputSampleL += (_bL[37] * (Float(0.11344930914138468) + (Float(0.17697142512225839) * etl::abs(_bL[37]))));
        inputSampleL += (_bL[38] * (Float(0.22766779627643968) + (Float(0.13645102964003858) * etl::abs(_bL[38]))));
        inputSampleL += (_bL[39] * (Float(0.38378309953638229) - (Float(0.01997653307333791) * etl::abs(_bL[39]))));
        inputSampleL += (_bL[40] * (Float(0.52789400804568076) - (Float(0.21409137428422448) * etl::abs(_bL[40]))));
        inputSampleL += (_bL[41] * (Float(0.55444630296938280) - (Float(0.32331980931576626) * etl::abs(_bL[41]))));
        inputSampleL += (_bL[42] * (Float(0.42333237669264601) - (Float(0.26855847463044280) * etl::abs(_bL[42]))));
        inputSampleL += (_bL[43] * (Float(0.21942831522035078) - (Float(0.12051365248820624) * etl::abs(_bL[43]))));
        inputSampleL -= (_bL[44] * (Float(0.00584169427830633) - (Float(0.03706970171280329) * etl::abs(_bL[44]))));
        inputSampleL -= (_bL[45] * (Float(0.24279799124660351) - (Float(0.17296440491477982) * etl::abs(_bL[45]))));
        inputSampleL -= (_bL[46] * (Float(0.40173760787507085) - (Float(0.21717989835163351) * etl::abs(_bL[46]))));
        inputSampleL -= (_bL[47] * (Float(0.43930035724188155) - (Float(0.16425928481378199) * etl::abs(_bL[47]))));
        inputSampleL -= (_bL[48] * (Float(0.41067765934041811) - (Float(0.10390115786636855) * etl::abs(_bL[48]))));
        inputSampleL -= (_bL[49] * (Float(0.34409235547165967) - (Float(0.07268159377411920) * etl::abs(_bL[49]))));
        inputSampleL -= (_bL[50] * (Float(0.26542883122568151) - (Float(0.05483457497365785) * etl::abs(_bL[50]))));
        inputSampleL -= (_bL[51] * (Float(0.22024754776138800) - (Float(0.06484897950087598) * etl::abs(_bL[51]))));
        inputSampleL -= (_bL[52] * (Float(0.20394367993632415) - (Float(0.08746309731952180) * etl::abs(_bL[52]))));
        inputSampleL -= (_bL[53] * (Float(0.17565242431124092) - (Float(0.07611309538078760) * etl::abs(_bL[53]))));
        inputSampleL -= (_bL[54] * (Float(0.10116623231246825) - (Float(0.00642818706295112) * etl::abs(_bL[54]))));
        inputSampleL -= (_bL[55] * (Float(0.00782648272053632) + (Float(0.08004141267685004) * etl::abs(_bL[55]))));
        inputSampleL += (_bL[56] * (Float(0.05059046006747323) - (Float(0.12436676387548490) * etl::abs(_bL[56]))));
        inputSampleL += (_bL[57] * (Float(0.06241531553254467) - (Float(0.11530779547021434) * etl::abs(_bL[57]))));
        inputSampleL += (_bL[58] * (Float(0.04952694587101836) - (Float(0.08340945324333944) * etl::abs(_bL[58]))));
        inputSampleL += (_bL[59] * (Float(0.00843873294401687) - (Float(0.03279659052562903) * etl::abs(_bL[59]))));
        inputSampleL -= (_bL[60] * (Float(0.05161338949440241) - (Float(0.03428181149163798) * etl::abs(_bL[60]))));
        inputSampleL -= (_bL[61] * (Float(0.08165520146902012) - (Float(0.08196746092283110) * etl::abs(_bL[61]))));
        inputSampleL -= (_bL[62] * (Float(0.06639532849935320) - (Float(0.09797462781896329) * etl::abs(_bL[62]))));
        inputSampleL -= (_bL[63] * (Float(0.02953430910661621) - (Float(0.09175612938515763) * etl::abs(_bL[63]))));
        inputSampleL += (_bL[64] * (Float(0.00741058547442938) + (Float(0.05442091048731967) * etl::abs(_bL[64]))));
        inputSampleL += (_bL[65] * (Float(0.01832866125391727) + (Float(0.00306243693643687) * etl::abs(_bL[65]))));
        inputSampleL += (_bL[66] * (Float(0.00526964230373573) - (Float(0.04364102661136410) * etl::abs(_bL[66]))));
        inputSampleL -= (_bL[67] * (Float(0.00300984373848200) + (Float(0.09742737841278880) * etl::abs(_bL[67]))));
        inputSampleL -= (_bL[68] * (Float(0.00413616769576694) + (Float(0.14380661694523073) * etl::abs(_bL[68]))));
        inputSampleL -= (_bL[69] * (Float(0.00588769034931419) + (Float(0.16012843578892538) * etl::abs(_bL[69]))));
        inputSampleL -= (_bL[70] * (Float(0.00688588239450581) + (Float(0.14074464279305798) * etl::abs(_bL[70]))));
        inputSampleL -= (_bL[71] * (Float(0.02277307992926315) + (Float(0.07914752191801366) * etl::abs(_bL[71]))));
        inputSampleL -= (_bL[72] * (Float(0.04627166091180877) - (Float(0.00192787268067208) * etl::abs(_bL[72]))));
        inputSampleL -= (_bL[73] * (Float(0.05562045897455786) - (Float(0.05932868727665747) * etl::abs(_bL[73]))));
        inputSampleL -= (_bL[74] * (Float(0.05134243784922165) - (Float(0.08245334798868090) * etl::abs(_bL[74]))));
        inputSampleL -= (_bL[75] * (Float(0.04719409472239919) - (Float(0.07498680629253825) * etl::abs(_bL[75]))));
        inputSampleL -= (_bL[76] * (Float(0.05889738914266415) - (Float(0.06116127018043697) * etl::abs(_bL[76]))));
        inputSampleL -= (_bL[77] * (Float(0.09428363535111127) - (Float(0.06535868867863834) * etl::abs(_bL[77]))));
        inputSampleL -= (_bL[78] * (Float(0.15181756953225126) - (Float(0.08982979655234427) * etl::abs(_bL[78]))));
        inputSampleL -= (_bL[79] * (Float(0.20878969456036670) - (Float(0.10761070891499538) * etl::abs(_bL[79]))));
        inputSampleL -= (_bL[80] * (Float(0.22647885581813790) - (Float(0.08462542510349125) * etl::abs(_bL[80]))));
        inputSampleL -= (_bL[81] * (Float(0.19723482443646323) - (Float(0.02665160920736287) * etl::abs(_bL[81]))));
        inputSampleL -= (_bL[82] * (Float(0.16441643451155163) + (Float(0.02314691954338197) * etl::abs(_bL[82]))));
        inputSampleL -= (_bL[83] * (Float(0.15201914054931515) + (Float(0.04424903493886839) * etl::abs(_bL[83]))));
        inputSampleL -= (_bL[84] * (Float(0.15454370641307855) + (Float(0.04223203797913008) * etl::abs(_bL[84]))));

        temp         = (inputSampleL + _smoothCabBl) * (Float(1) / Float(3));
        _smoothCabBl = inputSampleL;
        inputSampleL = temp * Float(0.25);

        randy = (_dist(_rng) * Float(0.057));
        drySampleL
            = ((((inputSampleL * (1 - randy)) + (_lastCabSampleL * randy)) * _wet) + (drySampleL * (Float(1) - _wet)))
            * _outputlevel;
        _lastCabSampleL = inputSampleL;
        inputSampleL    = drySampleL;  // cab L

        if (_cycleEnd == 4) {
            _lastRefL[0] = _lastRefL[4];                       // start from previous last
            _lastRefL[2] = (_lastRefL[0] + inputSampleL) / 2;  // half
            _lastRefL[1] = (_lastRefL[0] + _lastRefL[2]) / 2;  // one quarter
            _lastRefL[3] = (_lastRefL[2] + inputSampleL) / 2;  // three quarters
            _lastRefL[4] = inputSampleL;                       // full
        }
        if (_cycleEnd == 3) {
            _lastRefL[0] = _lastRefL[3];                                      // start from previous last
            _lastRefL[2] = (_lastRefL[0] + _lastRefL[0] + inputSampleL) / 3;  // third
            _lastRefL[1] = (_lastRefL[0] + inputSampleL + inputSampleL) / 3;  // two thirds
            _lastRefL[3] = inputSampleL;                                      // full
        }
        if (_cycleEnd == 2) {
            _lastRefL[0] = _lastRefL[2];                       // start from previous last
            _lastRefL[1] = (_lastRefL[0] + inputSampleL) / 2;  // half
            _lastRefL[2] = inputSampleL;                       // full
        }
        if (_cycleEnd == 1) {
            _lastRefL[0] = inputSampleL;
        }
        _cycle       = 0;  // reset
        inputSampleL = _lastRefL[_cycle];
    } else {
        inputSampleL = _lastRefL[_cycle];
        // we are going through our references now
    }
    switch (_cycleEnd)  // multi-pole average using lastRef[] variables
    {
        case 4:
            _lastRefL[8] = inputSampleL;
            inputSampleL = (inputSampleL + _lastRefL[7]) * 0.5;
            _lastRefL[7] = _lastRefL[8];  // continue, do not break
            [[fallthrough]];
        case 3:
            _lastRefL[8] = inputSampleL;
            inputSampleL = (inputSampleL + _lastRefL[6]) * 0.5;
            _lastRefL[6] = _lastRefL[8];  // continue, do not break
            [[fallthrough]];
        case 2:
            _lastRefL[8] = inputSampleL;
            inputSampleL = (inputSampleL + _lastRefL[5]) * 0.5;
            _lastRefL[5] = _lastRefL[8];  // continue, do not break
        case 1: break;                    // no further averaging
    }

    return inputSampleL;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsFireAmp<Float, URNG>::reset() -> void
{
    _lastSampleL  = 0.0;
    _storeSampleL = 0.0;
    _smoothAl     = 0.0;
    _smoothBl     = 0.0;
    _smoothCl     = 0.0;
    _smoothDl     = 0.0;
    _smoothEl     = 0.0;
    _smoothFl     = 0.0;
    _smoothGl     = 0.0;
    _smoothHl     = 0.0;
    _smoothIl     = 0.0;
    _smoothJl     = 0.0;
    _smoothKl     = 0.0;
    _smoothLl     = 0.0;
    _iirSampleAl  = 0.0;
    _iirSampleBl  = 0.0;
    _iirSampleCl  = 0.0;
    _iirSampleDl  = 0.0;
    _iirSampleEl  = 0.0;
    _iirSampleFl  = 0.0;
    _iirSampleGl  = 0.0;
    _iirSampleHl  = 0.0;
    _iirSampleIl  = 0.0;
    _iirSampleJl  = 0.0;
    _iirSampleKl  = 0.0;
    _iirSampleLl  = 0.0;
    _iirLowpassL  = 0.0;
    _iirSpkAl     = 0.0;
    _iirSpkBl     = 0.0;
    _iirSubL      = 0.0;

    for (int fcount = 0; fcount < 257; fcount++) {
        _oddL[fcount]  = 0.0;
        _evenL[fcount] = 0.0;
    }

    _count = 0;
    _flip  = false;  // amp

    for (auto& fcount : _bL) {
        fcount = 0;
    }
    _smoothCabAl    = 0.0;
    _smoothCabBl    = 0.0;
    _lastCabSampleL = 0.0;  // cab

    for (int fcount = 0; fcount < 9; fcount++) {
        _lastRefL[fcount] = 0.0;
    }
    _cycle = 0;  // undersampling

    for (int x = 0; x < FixTotal; x++) {
        _fixA[x] = 0.0;
        _fixB[x] = 0.0;
        _fixC[x] = 0.0;
        _fixD[x] = 0.0;
        _fixE[x] = 0.0;
        _fixF[x] = 0.0;
    }  // filtering
}

}  // namespace grit
