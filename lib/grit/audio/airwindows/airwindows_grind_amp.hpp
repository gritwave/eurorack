#pragma once

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

    AirWindowsGrindAmp() = default;
    explicit AirWindowsGrindAmp(seed_type seed);

    auto setParameter(Parameter parameter) -> void;
    auto setSampleRate(Float sampleRate) -> void;

    [[nodiscard]] auto operator()(Float x) -> Float;
    auto reset() -> void;

private:
    URNG _rng{42};
    etl::uniform_real_distribution<Float> _dist{Float(0), Float(1)};

    Parameter _parameter{};
    Float _sampleRate{};

    Float smoothA;
    Float smoothB;
    Float smoothC;
    Float smoothD;
    Float smoothE;
    Float smoothF;
    Float smoothG;
    Float smoothH;
    Float smoothI;
    Float smoothJ;
    Float smoothK;
    Float secondA;
    Float secondB;
    Float secondC;
    Float secondD;
    Float secondE;
    Float secondF;
    Float secondG;
    Float secondH;
    Float secondI;
    Float secondJ;
    Float secondK;
    Float thirdA;
    Float thirdB;
    Float thirdC;
    Float thirdD;
    Float thirdE;
    Float thirdF;
    Float thirdG;
    Float thirdH;
    Float thirdI;
    Float thirdJ;
    Float thirdK;
    Float iirSampleA;
    Float iirSampleB;
    Float iirSampleC;
    Float iirSampleD;
    Float iirSampleE;
    Float iirSampleF;
    Float iirSampleG;
    Float iirSampleH;
    Float iirSampleI;
    Float iirLowpass;
    Float iirSub;
    Float storeSample;  // amp

    Float bL[90];
    Float lastCabSample;
    Float smoothCabA;
    Float smoothCabB;  // cab

    Float lastRef[10];
    int cycle;  // undersampling

    // fixed frequency biquad filter for ultrasonics, stereo
    enum
    {
        fix_freq,
        fix_reso,
        fix_a0,
        fix_a1,
        fix_a2,
        fix_b1,
        fix_b2,
        fix_s1,
        fix_s2,
        fix_total
    };

    Float fixA[fix_total];
    Float fixB[fix_total];
    Float fixC[fix_total];
    Float fixD[fix_total];
    Float fixE[fix_total];
    Float fixF[fix_total];  // filtering
};

template<etl::floating_point Float, typename URNG>
AirWindowsGrindAmp<Float, URNG>::AirWindowsGrindAmp(seed_type seed) : _rng{seed}
{
    reset();
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsGrindAmp<Float, URNG>::setParameter(Parameter parameter) -> void
{
    _parameter = parameter;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsGrindAmp<Float, URNG>::setSampleRate(Float sampleRate) -> void
{
    _sampleRate = sampleRate;
    reset();
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsGrindAmp<Float, URNG>::operator()(Float const x) -> Float
{
    static constexpr auto const pi = static_cast<Float>(etl::numbers::pi);

    auto const A = _parameter.gain;
    auto const B = _parameter.tone;
    auto const C = _parameter.output;
    auto const D = _parameter.mix;

    Float overallscale = Float(1);
    overallscale /= 44100.0;
    overallscale *= _sampleRate;
    int cycleEnd = etl::floor(overallscale);
    if (cycleEnd < 1)
        cycleEnd = 1;
    if (cycleEnd > 4)
        cycleEnd = 4;
    // this is going to be 2 for 88.1 or 96k, 3 for silly people, 4 for 176 or 192k
    if (cycle > cycleEnd - 1)
        cycle = cycleEnd - 1;  // sanity check

    Float inputlevel = etl::pow(A, Float(2));
    Float samplerate = _sampleRate;
    Float trimEQ     = 1.1 - B;
    Float toneEQ     = trimEQ / 1.2;
    trimEQ /= 50.0;
    trimEQ += 0.165;
    Float EQ          = ((trimEQ - (toneEQ / Float(6.1))) / samplerate) * Float(22050);
    Float BEQ         = ((trimEQ + (toneEQ / Float(2.1))) / samplerate) * Float(22050);
    Float outputlevel = C;
    Float wet         = D;
    Float bassdrive   = Float(1.57079633) * (Float(2.5) - toneEQ);

    Float cutoff = (Float(18000) + (B * Float(1000))) / _sampleRate;
    if (cutoff > 0.49)
        cutoff = 0.49;  // don't crash if run at 44.1k
    if (cutoff < 0.001)
        cutoff = 0.001;  // or if cutoff's too low

    fixF[fix_freq] = fixE[fix_freq] = fixD[fix_freq] = fixC[fix_freq] = fixB[fix_freq] = fixA[fix_freq] = cutoff;

    fixA[fix_reso] = Float(4.46570214);
    fixB[fix_reso] = Float(1.51387132);
    fixC[fix_reso] = Float(0.93979296);
    fixD[fix_reso] = Float(0.70710678);
    fixE[fix_reso] = Float(0.52972649);
    fixF[fix_reso] = Float(0.50316379);

    Float K      = etl::tan(pi * fixA[fix_freq]);  // lowpass
    Float norm   = Float(1) / (Float(1) + K / fixA[fix_reso] + K * K);
    fixA[fix_a0] = K * K * norm;
    fixA[fix_a1] = Float(2) * fixA[fix_a0];
    fixA[fix_a2] = fixA[fix_a0];
    fixA[fix_b1] = Float(2) * (K * K - Float(1)) * norm;
    fixA[fix_b2] = (Float(1) - K / fixA[fix_reso] + K * K) * norm;

    K            = etl::tan(pi * fixB[fix_freq]);
    norm         = Float(1) / (Float(1) + K / fixB[fix_reso] + K * K);
    fixB[fix_a0] = K * K * norm;
    fixB[fix_a1] = Float(2) * fixB[fix_a0];
    fixB[fix_a2] = fixB[fix_a0];
    fixB[fix_b1] = Float(2) * (K * K - Float(1)) * norm;
    fixB[fix_b2] = (Float(1) - K / fixB[fix_reso] + K * K) * norm;

    K            = etl::tan(pi * fixC[fix_freq]);
    norm         = Float(1) / (Float(1) + K / fixC[fix_reso] + K * K);
    fixC[fix_a0] = K * K * norm;
    fixC[fix_a1] = Float(2) * fixC[fix_a0];
    fixC[fix_a2] = fixC[fix_a0];
    fixC[fix_b1] = Float(2) * (K * K - Float(1)) * norm;
    fixC[fix_b2] = (Float(1) - K / fixC[fix_reso] + K * K) * norm;

    K            = etl::tan(pi * fixD[fix_freq]);
    norm         = Float(1) / (Float(1) + K / fixD[fix_reso] + K * K);
    fixD[fix_a0] = K * K * norm;
    fixD[fix_a1] = Float(2) * fixD[fix_a0];
    fixD[fix_a2] = fixD[fix_a0];
    fixD[fix_b1] = Float(2) * (K * K - Float(1)) * norm;
    fixD[fix_b2] = (Float(1) - K / fixD[fix_reso] + K * K) * norm;

    K            = etl::tan(pi * fixE[fix_freq]);
    norm         = Float(1) / (Float(1) + K / fixE[fix_reso] + K * K);
    fixE[fix_a0] = K * K * norm;
    fixE[fix_a1] = Float(2) * fixE[fix_a0];
    fixE[fix_a2] = fixE[fix_a0];
    fixE[fix_b1] = Float(2) * (K * K - Float(1)) * norm;
    fixE[fix_b2] = (Float(1) - K / fixE[fix_reso] + K * K) * norm;

    K            = etl::tan(pi * fixF[fix_freq]);
    norm         = Float(1) / (Float(1) + K / fixF[fix_reso] + K * K);
    fixF[fix_a0] = K * K * norm;
    fixF[fix_a1] = Float(2) * fixF[fix_a0];
    fixF[fix_a2] = fixF[fix_a0];
    fixF[fix_b1] = Float(2) * (K * K - Float(1)) * norm;
    fixF[fix_b2] = (Float(1) - K / fixF[fix_reso] + K * K) * norm;

    auto input    = x;
    auto dryInput = input;

    auto outSample = (input * fixA[fix_a0]) + fixA[fix_s1];
    fixA[fix_s1]   = (input * fixA[fix_a1]) - (outSample * fixA[fix_b1]) + fixA[fix_s2];
    fixA[fix_s2]   = (input * fixA[fix_a2]) - (outSample * fixA[fix_b2]);
    input          = outSample;  // fixed biquad filtering ultrasonics

    input *= inputlevel;
    iirSampleA = (iirSampleA * (1.0 - EQ)) + (input * EQ);
    input      = input - (iirSampleA * 0.92);
    // highpass
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    auto bridgerectifier = etl::abs(input);
    auto inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier      = (smoothA + (secondA * inverse) + (thirdA * bridgerectifier) + input);
    thirdA               = secondA;
    secondA              = smoothA;
    smoothA              = input;
    auto basscatchL = input = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * fixB[fix_a0]) + fixB[fix_s1];
    fixB[fix_s1] = (input * fixB[fix_a1]) - (outSample * fixB[fix_b1]) + fixB[fix_s2];
    fixB[fix_s2] = (input * fixB[fix_a2]) - (outSample * fixB[fix_b2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    input *= inputlevel;
    iirSampleB = (iirSampleB * (1.0 - EQ)) + (input * EQ);
    input      = input - (iirSampleB * 0.79);
    // highpass
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    // overdrive
    bridgerectifier = etl::abs(input);
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothB + (secondB * inverse) + (thirdB * bridgerectifier) + input);
    thirdB          = secondB;
    secondB         = smoothB;
    smoothB         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    iirSampleC      = (iirSampleC * (1.0 - BEQ)) + (basscatchL * BEQ);
    basscatchL      = iirSampleC * bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633)
        bridgerectifier = 1.57079633;
    bridgerectifier = etl::sin(bridgerectifier);
    if (basscatchL > 0.0)
        basscatchL = bridgerectifier;
    else
        basscatchL = -bridgerectifier;
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    // overdrive
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothC + (secondC * inverse) + (thirdC * bridgerectifier) + input);
    thirdC          = secondC;
    secondC         = smoothC;
    smoothC         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * fixC[fix_a0]) + fixC[fix_s1];
    fixC[fix_s1] = (input * fixC[fix_a1]) - (outSample * fixC[fix_b1]) + fixC[fix_s2];
    fixC[fix_s2] = (input * fixC[fix_a2]) - (outSample * fixC[fix_b2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    iirSampleD      = (iirSampleD * (1.0 - BEQ)) + (basscatchL * BEQ);
    basscatchL      = iirSampleD * bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633)
        bridgerectifier = 1.57079633;
    bridgerectifier = etl::sin(bridgerectifier);
    if (basscatchL > 0.0)
        basscatchL = bridgerectifier;
    else
        basscatchL = -bridgerectifier;
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    // overdrive
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothD + (secondD * inverse) + (thirdD * bridgerectifier) + input);
    thirdD          = secondD;
    secondD         = smoothD;
    smoothD         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * fixD[fix_a0]) + fixD[fix_s1];
    fixD[fix_s1] = (input * fixD[fix_a1]) - (outSample * fixD[fix_b1]) + fixD[fix_s2];
    fixD[fix_s2] = (input * fixD[fix_a2]) - (outSample * fixD[fix_b2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    iirSampleE      = (iirSampleE * (1.0 - BEQ)) + (basscatchL * BEQ);
    basscatchL      = iirSampleE * bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633)
        bridgerectifier = 1.57079633;
    bridgerectifier = etl::sin(bridgerectifier);
    if (basscatchL > 0.0)
        basscatchL = bridgerectifier;
    else
        basscatchL = -bridgerectifier;
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    // overdrive
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothE + (secondE * inverse) + (thirdE * bridgerectifier) + input);
    thirdE          = secondE;
    secondE         = smoothE;
    smoothE         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    iirSampleF      = (iirSampleF * (1.0 - BEQ)) + (basscatchL * BEQ);
    basscatchL      = iirSampleF * bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633)
        bridgerectifier = 1.57079633;
    bridgerectifier = etl::sin(bridgerectifier);
    if (basscatchL > 0.0)
        basscatchL = bridgerectifier;
    else
        basscatchL = -bridgerectifier;
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    // overdrive
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothF + (secondF * inverse) + (thirdF * bridgerectifier) + input);
    thirdF          = secondF;
    secondF         = smoothF;
    smoothF         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * fixE[fix_a0]) + fixE[fix_s1];
    fixE[fix_s1] = (input * fixE[fix_a1]) - (outSample * fixE[fix_b1]) + fixE[fix_s2];
    fixE[fix_s2] = (input * fixE[fix_a2]) - (outSample * fixE[fix_b2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    iirSampleG      = (iirSampleG * (1.0 - BEQ)) + (basscatchL * BEQ);
    basscatchL      = iirSampleG * bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633)
        bridgerectifier = 1.57079633;
    bridgerectifier = etl::sin(bridgerectifier);
    if (basscatchL > 0.0)
        basscatchL = bridgerectifier;
    else
        basscatchL = -bridgerectifier;
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    // overdrive
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothG + (secondG * inverse) + (thirdG * bridgerectifier) + input);
    thirdG          = secondG;
    secondG         = smoothG;
    smoothG         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    iirSampleH      = (iirSampleH * (1.0 - BEQ)) + (basscatchL * BEQ);
    basscatchL      = iirSampleH * bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633)
        bridgerectifier = 1.57079633;
    bridgerectifier = etl::sin(bridgerectifier);
    if (basscatchL > 0.0)
        basscatchL = bridgerectifier;
    else
        basscatchL = -bridgerectifier;
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    // overdrive
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothH + (secondH * inverse) + (thirdH * bridgerectifier) + input);
    thirdH          = secondH;
    secondH         = smoothH;
    smoothH         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    outSample    = (input * fixF[fix_a0]) + fixF[fix_s1];
    fixF[fix_s1] = (input * fixF[fix_a1]) - (outSample * fixF[fix_b1]) + fixF[fix_s2];
    fixF[fix_s2] = (input * fixF[fix_a2]) - (outSample * fixF[fix_b2]);
    input        = outSample;  // fixed biquad filtering ultrasonics

    iirSampleI      = (iirSampleI * (1.0 - BEQ)) + (basscatchL * BEQ);
    basscatchL      = iirSampleI * bassdrive;
    bridgerectifier = etl::abs(basscatchL);
    if (bridgerectifier > 1.57079633)
        bridgerectifier = 1.57079633;
    bridgerectifier = etl::sin(bridgerectifier);
    if (basscatchL > 0.0)
        basscatchL = bridgerectifier;
    else
        basscatchL = -bridgerectifier;
    if (input > 1.0)
        input = 1.0;
    if (input < -1.0)
        input = -1.0;
    // overdrive
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothI + (secondI * inverse) + (thirdI * bridgerectifier) + input);
    thirdI          = secondI;
    secondI         = smoothI;
    smoothI         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass
    bridgerectifier = etl::abs(input);
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothJ + (secondJ * inverse) + (thirdJ * bridgerectifier) + input);
    thirdJ          = secondJ;
    secondJ         = smoothJ;
    smoothJ         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass
    bridgerectifier = etl::abs(input);
    inverse         = (bridgerectifier + 1.0) / 2.0;
    bridgerectifier = (smoothK + (secondK * inverse) + (thirdK * bridgerectifier) + input);
    thirdK          = secondK;
    secondK         = smoothK;
    smoothK         = input;
    input           = bridgerectifier;
    // three-sample averaging lowpass

    basscatchL /= 2.0;
    input = (input * toneEQ) + basscatchL;
    // extra lowpass for 4*12" speakers

    bridgerectifier = etl::abs(input * outputlevel);
    if (bridgerectifier > 1.57079633)
        bridgerectifier = 1.57079633;
    bridgerectifier = etl::sin(bridgerectifier);
    if (input > 0.0)
        input = bridgerectifier;
    else
        input = -bridgerectifier;
    input += basscatchL;
    // split bass between overdrive and clean
    input /= (1.0 + toneEQ);

    auto randy  = (_dist(_rng) * 0.061);
    input       = ((input * (1 - randy)) + (storeSample * randy)) * outputlevel;
    storeSample = input;

    if (wet != 1.0) {
        input = (input * wet) + (dryInput * (1.0 - wet));
    }
    // Dry/Wet control, defaults to the last slider
    // amp

    cycle++;
    if (cycle == cycleEnd) {

        auto temp  = (input + smoothCabA) / 3.0;
        smoothCabA = input;
        input      = temp;

        bL[83] = bL[82];
        bL[82] = bL[81];
        bL[81] = bL[80];
        bL[80] = bL[79];
        bL[79] = bL[78];
        bL[78] = bL[77];
        bL[77] = bL[76];
        bL[76] = bL[75];
        bL[75] = bL[74];
        bL[74] = bL[73];
        bL[73] = bL[72];
        bL[72] = bL[71];
        bL[71] = bL[70];
        bL[70] = bL[69];
        bL[69] = bL[68];
        bL[68] = bL[67];
        bL[67] = bL[66];
        bL[66] = bL[65];
        bL[65] = bL[64];
        bL[64] = bL[63];
        bL[63] = bL[62];
        bL[62] = bL[61];
        bL[61] = bL[60];
        bL[60] = bL[59];
        bL[59] = bL[58];
        bL[58] = bL[57];
        bL[57] = bL[56];
        bL[56] = bL[55];
        bL[55] = bL[54];
        bL[54] = bL[53];
        bL[53] = bL[52];
        bL[52] = bL[51];
        bL[51] = bL[50];
        bL[50] = bL[49];
        bL[49] = bL[48];
        bL[48] = bL[47];
        bL[47] = bL[46];
        bL[46] = bL[45];
        bL[45] = bL[44];
        bL[44] = bL[43];
        bL[43] = bL[42];
        bL[42] = bL[41];
        bL[41] = bL[40];
        bL[40] = bL[39];
        bL[39] = bL[38];
        bL[38] = bL[37];
        bL[37] = bL[36];
        bL[36] = bL[35];
        bL[35] = bL[34];
        bL[34] = bL[33];
        bL[33] = bL[32];
        bL[32] = bL[31];
        bL[31] = bL[30];
        bL[30] = bL[29];
        bL[29] = bL[28];
        bL[28] = bL[27];
        bL[27] = bL[26];
        bL[26] = bL[25];
        bL[25] = bL[24];
        bL[24] = bL[23];
        bL[23] = bL[22];
        bL[22] = bL[21];
        bL[21] = bL[20];
        bL[20] = bL[19];
        bL[19] = bL[18];
        bL[18] = bL[17];
        bL[17] = bL[16];
        bL[16] = bL[15];
        bL[15] = bL[14];
        bL[14] = bL[13];
        bL[13] = bL[12];
        bL[12] = bL[11];
        bL[11] = bL[10];
        bL[10] = bL[9];
        bL[9]  = bL[8];
        bL[8]  = bL[7];
        bL[7]  = bL[6];
        bL[6]  = bL[5];
        bL[5]  = bL[4];
        bL[4]  = bL[3];
        bL[3]  = bL[2];
        bL[2]  = bL[1];
        bL[1]  = bL[0];
        bL[0]  = input;
        input += (bL[1] * (1.29550481610475132 + (0.19713872057074355 * etl::abs(bL[1]))));
        input += (bL[2] * (1.42302569895462616 + (0.30599505521284787 * etl::abs(bL[2]))));
        input += (bL[3] * (1.28728195804197565 + (0.23168333460446133 * etl::abs(bL[3]))));
        input += (bL[4] * (0.88553784290822690 + (0.14263256172918892 * etl::abs(bL[4]))));
        input += (bL[5] * (0.37129054918432319 + (0.00150040944205920 * etl::abs(bL[5]))));
        input -= (bL[6] * (0.12150959412556320 + (0.32776273620569107 * etl::abs(bL[6]))));
        input -= (bL[7] * (0.44900065463203775 + (0.74101214925298819 * etl::abs(bL[7]))));
        input -= (bL[8] * (0.54058781908186482 + (1.07821707459008387 * etl::abs(bL[8]))));
        input -= (bL[9] * (0.49361966401791391 + (1.23540109014850508 * etl::abs(bL[9]))));
        input -= (bL[10] * (0.39819495093078133 + (1.11247213730917749 * etl::abs(bL[10]))));
        input -= (bL[11] * (0.31379279985435521 + (0.80330360359638298 * etl::abs(bL[11]))));
        input -= (bL[12] * (0.30744359242808555 + (0.42132528876858205 * etl::abs(bL[12]))));
        input -= (bL[13] * (0.33943170284673974 + (0.09183418349389982 * etl::abs(bL[13]))));
        input -= (bL[14] * (0.33838775119286391 - (0.06453051658561271 * etl::abs(bL[14]))));
        input -= (bL[15] * (0.30682305697961665 - (0.09549380253249232 * etl::abs(bL[15]))));
        input -= (bL[16] * (0.23408741339295336 - (0.08083404732361277 * etl::abs(bL[16]))));
        input -= (bL[17] * (0.10411746814025019 + (0.00253651281245780 * etl::abs(bL[17]))));
        input += (bL[18] * (0.00133623776084696 - (0.04447267870865820 * etl::abs(bL[18]))));
        input += (bL[19] * (0.02461903992114161 + (0.07530671732655550 * etl::abs(bL[19]))));
        input += (bL[20] * (0.02086715842475373 + (0.22795860236804899 * etl::abs(bL[20]))));
        input += (bL[21] * (0.02761433637100917 + (0.26108320417844094 * etl::abs(bL[21]))));
        input += (bL[22] * (0.04475285369162533 + (0.19160705011061663 * etl::abs(bL[22]))));
        input += (bL[23] * (0.09447338372862381 + (0.03681550508743799 * etl::abs(bL[23]))));
        input += (bL[24] * (0.13445890343722280 - (0.13713036462146147 * etl::abs(bL[24]))));
        input += (bL[25] * (0.13872868945088121 - (0.22401242373298191 * etl::abs(bL[25]))));
        input += (bL[26] * (0.14915650097434549 - (0.26718804981526367 * etl::abs(bL[26]))));
        input += (bL[27] * (0.12766643217091783 - (0.27745664795660430 * etl::abs(bL[27]))));
        input += (bL[28] * (0.03675849788393101 - (0.18338278173550679 * etl::abs(bL[28]))));
        input -= (bL[29] * (0.06307306864232835 + (0.06089480869040766 * etl::abs(bL[29]))));
        input -= (bL[30] * (0.14947389348962944 + (0.04642103054798480 * etl::abs(bL[30]))));
        input -= (bL[31] * (0.25235266566401526 + (0.08423062596460507 * etl::abs(bL[31]))));
        input -= (bL[32] * (0.33496344048679683 + (0.09808328256677995 * etl::abs(bL[32]))));
        input -= (bL[33] * (0.36590030482175445 + (0.10622650888958179 * etl::abs(bL[33]))));
        input -= (bL[34] * (0.35015197011464372 + (0.08982043516016047 * etl::abs(bL[34]))));
        input -= (bL[35] * (0.26808437585665090 + (0.00735561860229533 * etl::abs(bL[35]))));
        input -= (bL[36] * (0.11624318543291220 - (0.07142484314510467 * etl::abs(bL[36]))));
        input += (bL[37] * (0.05617084165377551 + (0.11785854050350089 * etl::abs(bL[37]))));
        input += (bL[38] * (0.20540028692589385 + (0.20479174663329586 * etl::abs(bL[38]))));
        input += (bL[39] * (0.30455415003043818 + (0.29074864580096849 * etl::abs(bL[39]))));
        input += (bL[40] * (0.33810750937829476 + (0.29182307921316802 * etl::abs(bL[40]))));
        input += (bL[41] * (0.31936133365277430 + (0.26535537727394987 * etl::abs(bL[41]))));
        input += (bL[42] * (0.27388548321981876 + (0.19735049990538350 * etl::abs(bL[42]))));
        input += (bL[43] * (0.21454597517994098 + (0.06415909270247236 * etl::abs(bL[43]))));
        input += (bL[44] * (0.15001045817707717 - (0.03831118543404573 * etl::abs(bL[44]))));
        input += (bL[45] * (0.07283437284653138 - (0.09281952429543777 * etl::abs(bL[45]))));
        input -= (bL[46] * (0.03917872184241358 + (0.14306291461398810 * etl::abs(bL[46]))));
        input -= (bL[47] * (0.16695932032148642 + (0.19138995946950504 * etl::abs(bL[47]))));
        input -= (bL[48] * (0.27055854466909462 + (0.22531296466343192 * etl::abs(bL[48]))));
        input -= (bL[49] * (0.33256357307578271 + (0.23305840475692102 * etl::abs(bL[49]))));
        input -= (bL[50] * (0.33459770116834442 + (0.24091822618917569 * etl::abs(bL[50]))));
        input -= (bL[51] * (0.27156687236338090 + (0.24062938573512443 * etl::abs(bL[51]))));
        input -= (bL[52] * (0.17197093288412094 + (0.19083085091993421 * etl::abs(bL[52]))));
        input -= (bL[53] * (0.06738628195910543 + (0.10268609751019808 * etl::abs(bL[53]))));
        input += (bL[54] * (0.00222429218204290 + (0.01439664435720548 * etl::abs(bL[54]))));
        input += (bL[55] * (0.01346992803494091 + (0.15947137113534526 * etl::abs(bL[55]))));
        input -= (bL[56] * (0.02038911881377448 - (0.26763170752416160 * etl::abs(bL[56]))));
        input -= (bL[57] * (0.08233579178189687 - (0.29415931086406055 * etl::abs(bL[57]))));
        input -= (bL[58] * (0.15447855089824883 - (0.26489186990840807 * etl::abs(bL[58]))));
        input -= (bL[59] * (0.20518281113362655 - (0.16135382257522859 * etl::abs(bL[59]))));
        input -= (bL[60] * (0.22244686050232007 + (0.00847180390247432 * etl::abs(bL[60]))));
        input -= (bL[61] * (0.21849243134998034 + (0.14460595245753741 * etl::abs(bL[61]))));
        input -= (bL[62] * (0.20256105734574054 + (0.18932793221831667 * etl::abs(bL[62]))));
        input -= (bL[63] * (0.18604070054295399 + (0.17250665610927965 * etl::abs(bL[63]))));
        input -= (bL[64] * (0.17222844322058231 + (0.12992472027850357 * etl::abs(bL[64]))));
        input -= (bL[65] * (0.14447856616566443 + (0.09089219002147308 * etl::abs(bL[65]))));
        input -= (bL[66] * (0.10385520794251019 + (0.08600465834570559 * etl::abs(bL[66]))));
        input -= (bL[67] * (0.07124435678265063 + (0.09071532210549428 * etl::abs(bL[67]))));
        input -= (bL[68] * (0.05216857461197572 + (0.06794061706070262 * etl::abs(bL[68]))));
        input -= (bL[69] * (0.05235381920184123 + (0.02818101717909346 * etl::abs(bL[69]))));
        input -= (bL[70] * (0.07569701245553526 - (0.00634228544764946 * etl::abs(bL[70]))));
        input -= (bL[71] * (0.10320125382718826 - (0.02751486906644141 * etl::abs(bL[71]))));
        input -= (bL[72] * (0.12122120969079088 - (0.05434007312178933 * etl::abs(bL[72]))));
        input -= (bL[73] * (0.13438969117200902 - (0.09135218559713874 * etl::abs(bL[73]))));
        input -= (bL[74] * (0.13534390437529981 - (0.10437672041458675 * etl::abs(bL[74]))));
        input -= (bL[75] * (0.11424128854188388 - (0.08693450726462598 * etl::abs(bL[75]))));
        input -= (bL[76] * (0.08166894518596159 - (0.06949989431475120 * etl::abs(bL[76]))));
        input -= (bL[77] * (0.04293976378555305 - (0.05718625137421843 * etl::abs(bL[77]))));
        input += (bL[78] * (0.00933076320644409 + (0.01728285211520138 * etl::abs(bL[78]))));
        input += (bL[79] * (0.06450430362918153 - (0.02492994833691022 * etl::abs(bL[79]))));
        input += (bL[80] * (0.10187400687649277 - (0.03578455940532403 * etl::abs(bL[80]))));
        input += (bL[81] * (0.11039763294094571 - (0.03995523517573508 * etl::abs(bL[81]))));
        input += (bL[82] * (0.08557960776024547 - (0.03482514309492527 * etl::abs(bL[82]))));
        input += (bL[83] * (0.02730881850805332 - (0.00514750108411127 * etl::abs(bL[83]))));

        temp       = (input + smoothCabB) / 3.0;
        smoothCabB = input;
        input      = temp / 4.0;

        randy    = (_dist(_rng) * 0.044);
        dryInput = ((((input * (1 - randy)) + (lastCabSample * randy)) * wet) + (dryInput * (1.0 - wet))) * outputlevel;
        lastCabSample = input;
        input         = dryInput;  // cab L

        if (cycleEnd == 4) {
            lastRef[0] = lastRef[4];                     // start from previous last
            lastRef[2] = (lastRef[0] + input) / 2;       // half
            lastRef[1] = (lastRef[0] + lastRef[2]) / 2;  // one quarter
            lastRef[3] = (lastRef[2] + input) / 2;       // three quarters
            lastRef[4] = input;                          // full
        }
        if (cycleEnd == 3) {
            lastRef[0] = lastRef[3];                             // start from previous last
            lastRef[2] = (lastRef[0] + lastRef[0] + input) / 3;  // thir
            lastRef[1] = (lastRef[0] + input + input) / 3;       // two third
            lastRef[3] = input;                                  // full
        }
        if (cycleEnd == 2) {
            lastRef[0] = lastRef[2];                // start from previous last
            lastRef[1] = (lastRef[0] + input) / 2;  // half
            lastRef[2] = input;                     // full
        }
        if (cycleEnd == 1) {
            lastRef[0] = input;
        }
        cycle = 0;  // reset
        input = lastRef[cycle];
    } else {
        input = lastRef[cycle];
        // we are going through our references now
    }
    switch (cycleEnd)  // multi-pole average using lastRef[] variables
    {
        case 4:
            lastRef[8] = input;
            input      = (input + lastRef[7]) * 0.5;
            lastRef[7] = lastRef[8];  // continue, do not break
            [[fallthrough]];
        case 3:
            lastRef[8] = input;
            input      = (input + lastRef[6]) * 0.5;
            lastRef[6] = lastRef[8];  // continue, do not break
            [[fallthrough]];
        case 2:
            lastRef[8] = input;
            input      = (input + lastRef[5]) * 0.5;
            lastRef[5] = lastRef[8];  // continue, do not break
        case 1: break;                // no further averaging
    }

    return input;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsGrindAmp<Float, URNG>::reset() -> void
{
    smoothA       = Float(0);
    smoothB       = Float(0);
    smoothC       = Float(0);
    smoothD       = Float(0);
    smoothE       = Float(0);
    smoothF       = Float(0);
    smoothG       = Float(0);
    smoothH       = Float(0);
    smoothI       = Float(0);
    smoothJ       = Float(0);
    smoothK       = Float(0);
    secondA       = Float(0);
    secondB       = Float(0);
    secondC       = Float(0);
    secondD       = Float(0);
    secondE       = Float(0);
    secondF       = Float(0);
    secondG       = Float(0);
    secondH       = Float(0);
    secondI       = Float(0);
    secondJ       = Float(0);
    secondK       = Float(0);
    thirdA        = Float(0);
    thirdB        = Float(0);
    thirdC        = Float(0);
    thirdD        = Float(0);
    thirdE        = Float(0);
    thirdF        = Float(0);
    thirdG        = Float(0);
    thirdH        = Float(0);
    thirdI        = Float(0);
    thirdJ        = Float(0);
    thirdK        = Float(0);
    iirSampleA    = Float(0);
    iirSampleB    = Float(0);
    iirSampleC    = Float(0);
    iirSampleD    = Float(0);
    iirSampleE    = Float(0);
    iirSampleF    = Float(0);
    iirSampleG    = Float(0);
    iirSampleH    = Float(0);
    iirSampleI    = Float(0);
    iirLowpass    = Float(0);
    iirSub        = Float(0);
    storeSample   = Float(0);  // amp
    smoothCabA    = Float(0);
    smoothCabB    = Float(0);
    lastCabSample = Float(0);  // cab
    cycle         = 0;         // undersampling

    for (int fcount = 0; fcount < 90; fcount++) {
        bL[fcount] = 0;
    }

    for (int fcount = 0; fcount < 9; fcount++) {
        lastRef[fcount] = Float(0);
    }
}

}  // namespace grit
