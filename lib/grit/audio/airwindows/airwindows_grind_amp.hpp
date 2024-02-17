#pragma once

#include <grit/math/static_lookup_table_transform.hpp>

#include <etl/algorithm.hpp>
#include <etl/cmath.hpp>
#include <etl/concepts.hpp>
#include <etl/random.hpp>

namespace grit {

/// \ingroup grit-audio-airwindows
template<etl::floating_point Float, typename URNG = etl::xoshiro128plusplus>
struct AirWindowsGrindAmp
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

    AirWindowsGrindAmp();
    explicit AirWindowsGrindAmp(seed_type seed);

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

    Float _smoothA{};
    Float _smoothB{};
    Float _smoothC{};
    Float _smoothD{};
    Float _smoothE{};
    Float _smoothF{};
    Float _smoothG{};
    Float _smoothH{};
    Float _smoothI{};
    Float _smoothJ{};
    Float _smoothK{};
    Float _secondA{};
    Float _secondB{};
    Float _secondC{};
    Float _secondD{};
    Float _secondE{};
    Float _secondF{};
    Float _secondG{};
    Float _secondH{};
    Float _secondI{};
    Float _secondJ{};
    Float _secondK{};
    Float _thirdA{};
    Float _thirdB{};
    Float _thirdC{};
    Float _thirdD{};
    Float _thirdE{};
    Float _thirdF{};
    Float _thirdG{};
    Float _thirdH{};
    Float _thirdI{};
    Float _thirdJ{};
    Float _thirdK{};
    Float _iirSampleA{};
    Float _iirSampleB{};
    Float _iirSampleC{};
    Float _iirSampleD{};
    Float _iirSampleE{};
    Float _iirSampleF{};
    Float _iirSampleG{};
    Float _iirSampleH{};
    Float _iirSampleI{};
    Float _iirLowpass{};
    Float _iirSub{};
    Float _storeSample{};  // amp

    Float _bL[90]{};
    Float _lastCabSample{};
    Float _smoothCabA{};
    Float _smoothCabB{};  // cab

    Float _lastRef[10]{};
    int _cycle{};  // undersampling

    // fixed frequency biquad filter for ultrasonics, stereo
    enum
    {
        FixFreq,
        FixReso,
        FixA0,
        FixA1,
        FixA2,
        FixB1,
        FixB2,
        FixS1,
        FixS2,
        FixTotal
    };

    Float _fixA[FixTotal]{};
    Float _fixB[FixTotal]{};
    Float _fixC[FixTotal]{};
    Float _fixD[FixTotal]{};
    Float _fixE[FixTotal]{};
    Float _fixF[FixTotal]{};  // filtering

    // Parameter Temporary
    Float _inputlevel{};
    Float _eq{};
    Float _beq{};
    Float _toneEq{};
    Float _trimEq{};
    Float _outputlevel{};
    Float _bassdrive{};
    Float _wet{};
    int _cycleEnd{};
};

template<etl::floating_point Float, typename URNG>
AirWindowsGrindAmp<Float, URNG>::AirWindowsGrindAmp()
{
    reset();
}

template<etl::floating_point Float, typename URNG>
AirWindowsGrindAmp<Float, URNG>::AirWindowsGrindAmp(seed_type seed) : _rng{seed}
{
    reset();
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsGrindAmp<Float, URNG>::setParameter(Parameter parameter) -> void
{
    _parameter = parameter;

    static constexpr auto const pi = static_cast<Float>(etl::numbers::pi);

    auto const a = _parameter.gain;
    auto const b = _parameter.tone;
    auto const c = _parameter.output;
    auto const d = _parameter.mix;

    Float overallscale = Float(1) / Float(44100.0);
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

    _inputlevel      = etl::pow(a, Float(2));
    Float samplerate = _sampleRate;
    _trimEq          = Float(1.1) - b;
    _toneEq          = _trimEq / Float(1.2);
    _trimEq /= Float(50.0);
    _trimEq += Float(0.165);
    _eq          = ((_trimEq - (_toneEq / Float(6.1))) / samplerate) * Float(22050);
    _beq         = ((_trimEq + (_toneEq / Float(2.1))) / samplerate) * Float(22050);
    _outputlevel = c;
    _wet         = d;
    _bassdrive   = Float(1.57079633) * (Float(2.5) - _toneEq);

    Float cutoff = (Float(18000) + (b * Float(1000))) / _sampleRate;
    if (cutoff > Float(0.49)) {
        cutoff = Float(0.49);  // don't crash if run at 44.1k
    }
    if (cutoff < Float(0.001)) {
        cutoff = Float(0.001);  // or if cutoff's too low
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
auto AirWindowsGrindAmp<Float, URNG>::setSampleRate(Float sampleRate) -> void
{
    _sampleRate = sampleRate;
    reset();
    setParameter(_parameter);
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsGrindAmp<Float, URNG>::operator()(Float const x) -> Float
{
    auto input    = x;
    auto dryInput = input;

    auto outSample = (input * _fixA[FixA0]) + _fixA[FixS1];
    _fixA[FixS1]   = (input * _fixA[FixA1]) - (outSample * _fixA[FixB1]) + _fixA[FixS2];
    _fixA[FixS2]   = (input * _fixA[FixA2]) - (outSample * _fixA[FixB2]);
    input          = outSample;  // fixed biquad filtering ultrasonics

    input *= _inputlevel;
    _iirSampleA = (_iirSampleA * (Float(1) - _eq)) + (input * _eq);
    input       = input - (_iirSampleA * 0.92);
    // highpass
    if (input > Float(1)) {
        input = Float(1);
    }
    if (input < Float(-1)) {
        input = Float(-1);
    }
    auto bridgerectifier = etl::abs(input);
    auto inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier      = (_smoothA + (_secondA * inverse) + (_thirdA * bridgerectifier) + input);
    _thirdA              = _secondA;
    _secondA             = _smoothA;
    _smoothA             = input;
    auto basscatchL = input = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * _fixB[FixA0]) + _fixB[FixS1];
    _fixB[FixS1] = (input * _fixB[FixA1]) - (outSample * _fixB[FixB1]) + _fixB[FixS2];
    _fixB[FixS2] = (input * _fixB[FixA2]) - (outSample * _fixB[FixB2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    input *= _inputlevel;
    _iirSampleB = (_iirSampleB * (Float(1) - _eq)) + (input * _eq);
    input       = input - (_iirSampleB * 0.79);
    // highpass
    if (input > Float(1)) {
        input = Float(1);
    }
    if (input < Float(-1)) {
        input = Float(-1);
    }
    // overdrive
    bridgerectifier = etl::abs(input);
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothB + (_secondB * inverse) + (_thirdB * bridgerectifier) + input);
    _thirdB         = _secondB;
    _secondB        = _smoothB;
    _smoothB        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    _iirSampleC     = (_iirSampleC * (Float(1) - _beq)) + (basscatchL * _beq);
    basscatchL      = _iirSampleC * _bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633) {
        bridgerectifier = 1.57079633;
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (basscatchL > 0.0) {
        basscatchL = bridgerectifier;
    } else {
        basscatchL = -bridgerectifier;
    }
    if (input > Float(1)) {
        input = Float(1);
    }
    if (input < Float(-1)) {
        input = Float(-1);
    }
    // overdrive
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothC + (_secondC * inverse) + (_thirdC * bridgerectifier) + input);
    _thirdC         = _secondC;
    _secondC        = _smoothC;
    _smoothC        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * _fixC[FixA0]) + _fixC[FixS1];
    _fixC[FixS1] = (input * _fixC[FixA1]) - (outSample * _fixC[FixB1]) + _fixC[FixS2];
    _fixC[FixS2] = (input * _fixC[FixA2]) - (outSample * _fixC[FixB2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    _iirSampleD     = (_iirSampleD * (Float(1) - _beq)) + (basscatchL * _beq);
    basscatchL      = _iirSampleD * _bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633) {
        bridgerectifier = 1.57079633;
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (basscatchL > Float(0.0)) {
        basscatchL = bridgerectifier;
    } else {
        basscatchL = -bridgerectifier;
    }
    if (input > Float(1.0)) {
        input = Float(1.0);
    }
    if (input < Float(-1.0)) {
        input = Float(-1.0);
    }
    // overdrive
    inverse         = (bridgerectifier + Float(1.0)) * Float(0.5);
    bridgerectifier = (_smoothD + (_secondD * inverse) + (_thirdD * bridgerectifier) + input);
    _thirdD         = _secondD;
    _secondD        = _smoothD;
    _smoothD        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * _fixD[FixA0]) + _fixD[FixS1];
    _fixD[FixS1] = (input * _fixD[FixA1]) - (outSample * _fixD[FixB1]) + _fixD[FixS2];
    _fixD[FixS2] = (input * _fixD[FixA2]) - (outSample * _fixD[FixB2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    _iirSampleE     = (_iirSampleE * (1.0 - _beq)) + (basscatchL * _beq);
    basscatchL      = _iirSampleE * _bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633) {
        bridgerectifier = 1.57079633;
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (basscatchL > 0.0) {
        basscatchL = bridgerectifier;
    } else {
        basscatchL = -bridgerectifier;
    }
    if (input > 1.0) {
        input = 1.0;
    }
    if (input < -1.0) {
        input = -1.0;
    }
    // overdrive
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothE + (_secondE * inverse) + (_thirdE * bridgerectifier) + input);
    _thirdE         = _secondE;
    _secondE        = _smoothE;
    _smoothE        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    _iirSampleF     = (_iirSampleF * (Float(1) - _beq)) + (basscatchL * _beq);
    basscatchL      = _iirSampleF * _bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633) {
        bridgerectifier = 1.57079633;
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (basscatchL > 0.0) {
        basscatchL = bridgerectifier;
    } else {
        basscatchL = -bridgerectifier;
    }
    if (input > 1.0) {
        input = 1.0;
    }
    if (input < -1.0) {
        input = -1.0;
    }
    // overdrive
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothF + (_secondF * inverse) + (_thirdF * bridgerectifier) + input);
    _thirdF         = _secondF;
    _secondF        = _smoothF;
    _smoothF        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * _fixE[FixA0]) + _fixE[FixS1];
    _fixE[FixS1] = (input * _fixE[FixA1]) - (outSample * _fixE[FixB1]) + _fixE[FixS2];
    _fixE[FixS2] = (input * _fixE[FixA2]) - (outSample * _fixE[FixB2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    _iirSampleG     = (_iirSampleG * (1.0 - _beq)) + (basscatchL * _beq);
    basscatchL      = _iirSampleG * _bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633) {
        bridgerectifier = 1.57079633;
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (basscatchL > 0.0) {
        basscatchL = bridgerectifier;
    } else {
        basscatchL = -bridgerectifier;
    }
    if (input > 1.0) {
        input = 1.0;
    }
    if (input < -1.0) {
        input = -1.0;
    }
    // overdrive
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothG + (_secondG * inverse) + (_thirdG * bridgerectifier) + input);
    _thirdG         = _secondG;
    _secondG        = _smoothG;
    _smoothG        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    _iirSampleH     = (_iirSampleH * (Float(1) - _beq)) + (basscatchL * _beq);
    basscatchL      = _iirSampleH * _bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633) {
        bridgerectifier = 1.57079633;
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (basscatchL > 0.0) {
        basscatchL = bridgerectifier;
    } else {
        basscatchL = -bridgerectifier;
    }
    if (input > Float(1)) {
        input = Float(1);
    }
    if (input < Float(-1)) {
        input = Float(-1);
    }
    // overdrive
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothH + (_secondH * inverse) + (_thirdH * bridgerectifier) + input);
    _thirdH         = _secondH;
    _secondH        = _smoothH;
    _smoothH        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * _fixF[FixA0]) + _fixF[FixS1];
    _fixF[FixS1] = (input * _fixF[FixA1]) - (outSample * _fixF[FixB1]) + _fixF[FixS2];
    _fixF[FixS2] = (input * _fixF[FixA2]) - (outSample * _fixF[FixB2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    _iirSampleI     = (_iirSampleI * (Float(1) - _beq)) + (basscatchL * _beq);
    basscatchL      = _iirSampleI * _bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633) {
        bridgerectifier = 1.57079633;
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (basscatchL > 0.0) {
        basscatchL = bridgerectifier;
    } else {
        basscatchL = -bridgerectifier;
    }
    if (input > Float(1)) {
        input = Float(1);
    }
    if (input < Float(-1)) {
        input = Float(-1);
    }
    // overdrive
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothI + (_secondI * inverse) + (_thirdI * bridgerectifier) + input);
    _thirdI         = _secondI;
    _secondI        = _smoothI;
    _smoothI        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass
    bridgerectifier = etl::abs(input);
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothJ + (_secondJ * inverse) + (_thirdJ * bridgerectifier) + input);
    _thirdJ         = _secondJ;
    _secondJ        = _smoothJ;
    _smoothJ        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass
    bridgerectifier = etl::abs(input);
    inverse         = (bridgerectifier + Float(1)) * Float(0.5);
    bridgerectifier = (_smoothK + (_secondK * inverse) + (_thirdK * bridgerectifier) + input);
    _thirdK         = _secondK;
    _secondK        = _smoothK;
    _smoothK        = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    basscatchL *= Float(0.5);
    input = (input * _toneEq) + basscatchL;
    // extra lowpass for 4*12" speakers

    bridgerectifier = etl::abs(input * _outputlevel);
    if (bridgerectifier > 1.57079633) {
        bridgerectifier = 1.57079633;
    }
    bridgerectifier = sineLUT(bridgerectifier);
    if (input > 0.0) {
        input = bridgerectifier;
    } else {
        input = -bridgerectifier;
    }
    input += basscatchL;
    // split bass between overdrive and clean
    input /= (Float(1) + _toneEq);

    auto randy   = (_dist(_rng) * Float(0.061));
    input        = ((input * (1 - randy)) + (_storeSample * randy)) * _outputlevel;
    _storeSample = input;

    if (_wet != Float(1)) {
        input = (input * _wet) + (dryInput * (Float(1) - _wet));
    }
    // Dry/Wet control, defaults to the last slider
    // amp

    _cycle++;
    if (_cycle == _cycleEnd) {

        auto temp   = (input + _smoothCabA) / Float(3);
        _smoothCabA = input;
        input       = temp;

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
        _bL[0]  = input;
        input += (_bL[1] * (Float(1.29550481610475132) + (Float(0.19713872057074355) * etl::abs(_bL[1]))));
        input += (_bL[2] * (Float(1.42302569895462616) + (Float(0.30599505521284787) * etl::abs(_bL[2]))));
        input += (_bL[3] * (Float(1.28728195804197565) + (Float(0.23168333460446133) * etl::abs(_bL[3]))));
        input += (_bL[4] * (Float(0.88553784290822690) + (Float(0.14263256172918892) * etl::abs(_bL[4]))));
        input += (_bL[5] * (Float(0.37129054918432319) + (Float(0.00150040944205920) * etl::abs(_bL[5]))));
        input -= (_bL[6] * (Float(0.12150959412556320) + (Float(0.32776273620569107) * etl::abs(_bL[6]))));
        input -= (_bL[7] * (Float(0.44900065463203775) + (Float(0.74101214925298819) * etl::abs(_bL[7]))));
        input -= (_bL[8] * (Float(0.54058781908186482) + (Float(1.07821707459008387) * etl::abs(_bL[8]))));
        input -= (_bL[9] * (Float(0.49361966401791391) + (Float(1.23540109014850508) * etl::abs(_bL[9]))));
        input -= (_bL[10] * (Float(0.39819495093078133) + (Float(1.11247213730917749) * etl::abs(_bL[10]))));
        input -= (_bL[11] * (Float(0.31379279985435521) + (Float(0.80330360359638298) * etl::abs(_bL[11]))));
        input -= (_bL[12] * (Float(0.30744359242808555) + (Float(0.42132528876858205) * etl::abs(_bL[12]))));
        input -= (_bL[13] * (Float(0.33943170284673974) + (Float(0.09183418349389982) * etl::abs(_bL[13]))));
        input -= (_bL[14] * (Float(0.33838775119286391) - (Float(0.06453051658561271) * etl::abs(_bL[14]))));
        input -= (_bL[15] * (Float(0.30682305697961665) - (Float(0.09549380253249232) * etl::abs(_bL[15]))));
        input -= (_bL[16] * (Float(0.23408741339295336) - (Float(0.08083404732361277) * etl::abs(_bL[16]))));
        input -= (_bL[17] * (Float(0.10411746814025019) + (Float(0.00253651281245780) * etl::abs(_bL[17]))));
        input += (_bL[18] * (Float(0.00133623776084696) - (Float(0.04447267870865820) * etl::abs(_bL[18]))));
        input += (_bL[19] * (Float(0.02461903992114161) + (Float(0.07530671732655550) * etl::abs(_bL[19]))));
        input += (_bL[20] * (Float(0.02086715842475373) + (Float(0.22795860236804899) * etl::abs(_bL[20]))));
        input += (_bL[21] * (Float(0.02761433637100917) + (Float(0.26108320417844094) * etl::abs(_bL[21]))));
        input += (_bL[22] * (Float(0.04475285369162533) + (Float(0.19160705011061663) * etl::abs(_bL[22]))));
        input += (_bL[23] * (Float(0.09447338372862381) + (Float(0.03681550508743799) * etl::abs(_bL[23]))));
        input += (_bL[24] * (Float(0.13445890343722280) - (Float(0.13713036462146147) * etl::abs(_bL[24]))));
        input += (_bL[25] * (Float(0.13872868945088121) - (Float(0.22401242373298191) * etl::abs(_bL[25]))));
        input += (_bL[26] * (Float(0.14915650097434549) - (Float(0.26718804981526367) * etl::abs(_bL[26]))));
        input += (_bL[27] * (Float(0.12766643217091783) - (Float(0.27745664795660430) * etl::abs(_bL[27]))));
        input += (_bL[28] * (Float(0.03675849788393101) - (Float(0.18338278173550679) * etl::abs(_bL[28]))));
        input -= (_bL[29] * (Float(0.06307306864232835) + (Float(0.06089480869040766) * etl::abs(_bL[29]))));
        input -= (_bL[30] * (Float(0.14947389348962944) + (Float(0.04642103054798480) * etl::abs(_bL[30]))));
        input -= (_bL[31] * (Float(0.25235266566401526) + (Float(0.08423062596460507) * etl::abs(_bL[31]))));
        input -= (_bL[32] * (Float(0.33496344048679683) + (Float(0.09808328256677995) * etl::abs(_bL[32]))));
        input -= (_bL[33] * (Float(0.36590030482175445) + (Float(0.10622650888958179) * etl::abs(_bL[33]))));
        input -= (_bL[34] * (Float(0.35015197011464372) + (Float(0.08982043516016047) * etl::abs(_bL[34]))));
        input -= (_bL[35] * (Float(0.26808437585665090) + (Float(0.00735561860229533) * etl::abs(_bL[35]))));
        input -= (_bL[36] * (Float(0.11624318543291220) - (Float(0.07142484314510467) * etl::abs(_bL[36]))));
        input += (_bL[37] * (Float(0.05617084165377551) + (Float(0.11785854050350089) * etl::abs(_bL[37]))));
        input += (_bL[38] * (Float(0.20540028692589385) + (Float(0.20479174663329586) * etl::abs(_bL[38]))));
        input += (_bL[39] * (Float(0.30455415003043818) + (Float(0.29074864580096849) * etl::abs(_bL[39]))));
        input += (_bL[40] * (Float(0.33810750937829476) + (Float(0.29182307921316802) * etl::abs(_bL[40]))));
        input += (_bL[41] * (Float(0.31936133365277430) + (Float(0.26535537727394987) * etl::abs(_bL[41]))));
        input += (_bL[42] * (Float(0.27388548321981876) + (Float(0.19735049990538350) * etl::abs(_bL[42]))));
        input += (_bL[43] * (Float(0.21454597517994098) + (Float(0.06415909270247236) * etl::abs(_bL[43]))));
        input += (_bL[44] * (Float(0.15001045817707717) - (Float(0.03831118543404573) * etl::abs(_bL[44]))));
        input += (_bL[45] * (Float(0.07283437284653138) - (Float(0.09281952429543777) * etl::abs(_bL[45]))));
        input -= (_bL[46] * (Float(0.03917872184241358) + (Float(0.14306291461398810) * etl::abs(_bL[46]))));
        input -= (_bL[47] * (Float(0.16695932032148642) + (Float(0.19138995946950504) * etl::abs(_bL[47]))));
        input -= (_bL[48] * (Float(0.27055854466909462) + (Float(0.22531296466343192) * etl::abs(_bL[48]))));
        input -= (_bL[49] * (Float(0.33256357307578271) + (Float(0.23305840475692102) * etl::abs(_bL[49]))));
        input -= (_bL[50] * (Float(0.33459770116834442) + (Float(0.24091822618917569) * etl::abs(_bL[50]))));
        input -= (_bL[51] * (Float(0.27156687236338090) + (Float(0.24062938573512443) * etl::abs(_bL[51]))));
        input -= (_bL[52] * (Float(0.17197093288412094) + (Float(0.19083085091993421) * etl::abs(_bL[52]))));
        input -= (_bL[53] * (Float(0.06738628195910543) + (Float(0.10268609751019808) * etl::abs(_bL[53]))));
        input += (_bL[54] * (Float(0.00222429218204290) + (Float(0.01439664435720548) * etl::abs(_bL[54]))));
        input += (_bL[55] * (Float(0.01346992803494091) + (Float(0.15947137113534526) * etl::abs(_bL[55]))));
        input -= (_bL[56] * (Float(0.02038911881377448) - (Float(0.26763170752416160) * etl::abs(_bL[56]))));
        input -= (_bL[57] * (Float(0.08233579178189687) - (Float(0.29415931086406055) * etl::abs(_bL[57]))));
        input -= (_bL[58] * (Float(0.15447855089824883) - (Float(0.26489186990840807) * etl::abs(_bL[58]))));
        input -= (_bL[59] * (Float(0.20518281113362655) - (Float(0.16135382257522859) * etl::abs(_bL[59]))));
        input -= (_bL[60] * (Float(0.22244686050232007) + (Float(0.00847180390247432) * etl::abs(_bL[60]))));
        input -= (_bL[61] * (Float(0.21849243134998034) + (Float(0.14460595245753741) * etl::abs(_bL[61]))));
        input -= (_bL[62] * (Float(0.20256105734574054) + (Float(0.18932793221831667) * etl::abs(_bL[62]))));
        input -= (_bL[63] * (Float(0.18604070054295399) + (Float(0.17250665610927965) * etl::abs(_bL[63]))));
        input -= (_bL[64] * (Float(0.17222844322058231) + (Float(0.12992472027850357) * etl::abs(_bL[64]))));
        input -= (_bL[65] * (Float(0.14447856616566443) + (Float(0.09089219002147308) * etl::abs(_bL[65]))));
        input -= (_bL[66] * (Float(0.10385520794251019) + (Float(0.08600465834570559) * etl::abs(_bL[66]))));
        input -= (_bL[67] * (Float(0.07124435678265063) + (Float(0.09071532210549428) * etl::abs(_bL[67]))));
        input -= (_bL[68] * (Float(0.05216857461197572) + (Float(0.06794061706070262) * etl::abs(_bL[68]))));
        input -= (_bL[69] * (Float(0.05235381920184123) + (Float(0.02818101717909346) * etl::abs(_bL[69]))));
        input -= (_bL[70] * (Float(0.07569701245553526) - (Float(0.00634228544764946) * etl::abs(_bL[70]))));
        input -= (_bL[71] * (Float(0.10320125382718826) - (Float(0.02751486906644141) * etl::abs(_bL[71]))));
        input -= (_bL[72] * (Float(0.12122120969079088) - (Float(0.05434007312178933) * etl::abs(_bL[72]))));
        input -= (_bL[73] * (Float(0.13438969117200902) - (Float(0.09135218559713874) * etl::abs(_bL[73]))));
        input -= (_bL[74] * (Float(0.13534390437529981) - (Float(0.10437672041458675) * etl::abs(_bL[74]))));
        input -= (_bL[75] * (Float(0.11424128854188388) - (Float(0.08693450726462598) * etl::abs(_bL[75]))));
        input -= (_bL[76] * (Float(0.08166894518596159) - (Float(0.06949989431475120) * etl::abs(_bL[76]))));
        input -= (_bL[77] * (Float(0.04293976378555305) - (Float(0.05718625137421843) * etl::abs(_bL[77]))));
        input += (_bL[78] * (Float(0.00933076320644409) + (Float(0.01728285211520138) * etl::abs(_bL[78]))));
        input += (_bL[79] * (Float(0.06450430362918153) - (Float(0.02492994833691022) * etl::abs(_bL[79]))));
        input += (_bL[80] * (Float(0.10187400687649277) - (Float(0.03578455940532403) * etl::abs(_bL[80]))));
        input += (_bL[81] * (Float(0.11039763294094571) - (Float(0.03995523517573508) * etl::abs(_bL[81]))));
        input += (_bL[82] * (Float(0.08557960776024547) - (Float(0.03482514309492527) * etl::abs(_bL[82]))));
        input += (_bL[83] * (Float(0.02730881850805332) - (Float(0.00514750108411127) * etl::abs(_bL[83]))));

        temp        = (input + _smoothCabB) / Float(3);
        _smoothCabB = input;
        input       = temp / Float(4);

        randy    = (_dist(_rng) * Float(0.044));
        dryInput = ((((input * (1 - randy)) + (_lastCabSample * randy)) * _wet) + (dryInput * (Float(1) - _wet)))
                 * _outputlevel;
        _lastCabSample = input;
        input          = dryInput;  // cab L

        if (_cycleEnd == 4) {
            _lastRef[0] = _lastRef[4];                      // start from previous last
            _lastRef[2] = (_lastRef[0] + input) / 2;        // half
            _lastRef[1] = (_lastRef[0] + _lastRef[2]) / 2;  // one quarter
            _lastRef[3] = (_lastRef[2] + input) / 2;        // three quarters
            _lastRef[4] = input;                            // full
        }
        if (_cycleEnd == 3) {
            _lastRef[0] = _lastRef[3];                              // start from previous last
            _lastRef[2] = (_lastRef[0] + _lastRef[0] + input) / 3;  // thir
            _lastRef[1] = (_lastRef[0] + input + input) / 3;        // two third
            _lastRef[3] = input;                                    // full
        }
        if (_cycleEnd == 2) {
            _lastRef[0] = _lastRef[2];                // start from previous last
            _lastRef[1] = (_lastRef[0] + input) / 2;  // half
            _lastRef[2] = input;                      // full
        }
        if (_cycleEnd == 1) {
            _lastRef[0] = input;
        }
        _cycle = 0;  // reset
        input  = _lastRef[_cycle];
    } else {
        input = _lastRef[_cycle];
        // we are going through our references now
    }
    switch (_cycleEnd)  // multi-pole average using lastRef[] variables
    {
        case 4:
            _lastRef[8] = input;
            input       = (input + _lastRef[7]) * Float(0.5);
            _lastRef[7] = _lastRef[8];  // continue, do not break
            [[fallthrough]];
        case 3:
            _lastRef[8] = input;
            input       = (input + _lastRef[6]) * Float(0.5);
            _lastRef[6] = _lastRef[8];  // continue, do not break
            [[fallthrough]];
        case 2:
            _lastRef[8] = input;
            input       = (input + _lastRef[5]) * Float(0.5);
            _lastRef[5] = _lastRef[8];  // continue, do not break
        case 1: break;                  // no further averaging
    }

    return input;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsGrindAmp<Float, URNG>::reset() -> void
{
    _smoothA       = Float(0);
    _smoothB       = Float(0);
    _smoothC       = Float(0);
    _smoothD       = Float(0);
    _smoothE       = Float(0);
    _smoothF       = Float(0);
    _smoothG       = Float(0);
    _smoothH       = Float(0);
    _smoothI       = Float(0);
    _smoothJ       = Float(0);
    _smoothK       = Float(0);
    _secondA       = Float(0);
    _secondB       = Float(0);
    _secondC       = Float(0);
    _secondD       = Float(0);
    _secondE       = Float(0);
    _secondF       = Float(0);
    _secondG       = Float(0);
    _secondH       = Float(0);
    _secondI       = Float(0);
    _secondJ       = Float(0);
    _secondK       = Float(0);
    _thirdA        = Float(0);
    _thirdB        = Float(0);
    _thirdC        = Float(0);
    _thirdD        = Float(0);
    _thirdE        = Float(0);
    _thirdF        = Float(0);
    _thirdG        = Float(0);
    _thirdH        = Float(0);
    _thirdI        = Float(0);
    _thirdJ        = Float(0);
    _thirdK        = Float(0);
    _iirSampleA    = Float(0);
    _iirSampleB    = Float(0);
    _iirSampleC    = Float(0);
    _iirSampleD    = Float(0);
    _iirSampleE    = Float(0);
    _iirSampleF    = Float(0);
    _iirSampleG    = Float(0);
    _iirSampleH    = Float(0);
    _iirSampleI    = Float(0);
    _iirLowpass    = Float(0);
    _iirSub        = Float(0);
    _storeSample   = Float(0);  // amp
    _smoothCabA    = Float(0);
    _smoothCabB    = Float(0);
    _lastCabSample = Float(0);  // cab
    _cycle         = 0;         // undersampling

    for (auto& fcount : _bL) {
        fcount = 0;
    }

    for (int fcount = 0; fcount < 9; fcount++) {
        _lastRef[fcount] = Float(0);
    }
}

}  // namespace grit
