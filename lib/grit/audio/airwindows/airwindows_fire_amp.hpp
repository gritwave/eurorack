#pragma once

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
    URNG _rng{42};
    etl::uniform_real_distribution<Float> _dist{Float(0), Float(1)};

    Parameter _parameter{};
    Float _sampleRate{};

    Float lastSampleL{0};
    Float storeSampleL{0};
    Float smoothAL{0};
    Float smoothBL{0};
    Float smoothCL{0};
    Float smoothDL{0};
    Float smoothEL{0};
    Float smoothFL{0};
    Float smoothGL{0};
    Float smoothHL{0};
    Float smoothIL{0};
    Float smoothJL{0};
    Float smoothKL{0};
    Float smoothLL{0};
    Float iirSampleAL{0};
    Float iirSampleBL{0};
    Float iirSampleCL{0};
    Float iirSampleDL{0};
    Float iirSampleEL{0};
    Float iirSampleFL{0};
    Float iirSampleGL{0};
    Float iirSampleHL{0};
    Float iirSampleIL{0};
    Float iirSampleJL{0};
    Float iirSampleKL{0};
    Float iirSampleLL{0};
    Float iirLowpassL{0};
    Float iirSpkAL{0};
    Float iirSpkBL{0};
    Float iirSubL{0};
    Float OddL[257]{0};
    Float EvenL[257]{0};

    bool flip{false};
    int count{0};  // amp

    Float bL[90]{0};
    Float lastCabSampleL{0};
    Float smoothCabAL{0};
    Float smoothCabBL{0};  // cab

    Float lastRefL[10]{0};
    int cycle{0};  // undersampling

    enum
    {
        fix_freq,
        fix_reso,
        fix_a0,
        fix_a1,
        fix_a2,
        fix_b1,
        fix_b2,
        fix_sL1,
        fix_sL2,
        fix_total
    };

    Float fixA[fix_total]{0};
    Float fixB[fix_total]{0};
    Float fixC[fix_total]{0};
    Float fixD[fix_total]{0};
    Float fixE[fix_total]{0};
    Float fixF[fix_total]{0};  // filtering
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
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsFireAmp<Float, URNG>::setSampleRate(Float sampleRate) -> void
{
    _sampleRate = sampleRate;
    reset();
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsFireAmp<Float, URNG>::operator()(Float const x) -> Float
{
    static constexpr auto const pi = static_cast<Float>(etl::numbers::pi);

    auto const A = _parameter.gain;
    auto const B = _parameter.tone;
    auto const C = _parameter.output;
    auto const D = _parameter.mix;

    Float bassfill    = A;
    Float outputlevel = C;
    Float wet         = D;

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

    Float startlevel = bassfill;
    Float samplerate = _sampleRate;
    Float basstrim   = bassfill / 16.0;
    Float toneEQ     = (B / samplerate) * 22050.0;
    Float EQ         = (basstrim / samplerate) * 22050.0;
    Float bleed      = outputlevel / 16.0;
    Float bassfactor = Float(1) - (basstrim * basstrim);
    Float BEQ        = (bleed / samplerate) * 22050.0;
    int diagonal     = (int)(0.000861678 * samplerate);
    if (diagonal > 127)
        diagonal = 127;
    int side = (int)(diagonal / 1.4142135623730951);
    int down = (side + diagonal) / 2;
    // now we've got down, side and diagonal as offsets and we also use three successive samples upfront

    Float cutoff = (15000.0 + (B * 10000.0)) / _sampleRate;
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

    auto inputSampleL = x;
    auto drySampleL   = inputSampleL;

    auto outSample = (inputSampleL * fixA[fix_a0]) + fixA[fix_sL1];
    fixA[fix_sL1]  = (inputSampleL * fixA[fix_a1]) - (outSample * fixA[fix_b1]) + fixA[fix_sL2];
    fixA[fix_sL2]  = (inputSampleL * fixA[fix_a2]) - (outSample * fixA[fix_b2]);
    inputSampleL   = outSample;  // fixed biquad filtering ultrasonics

    if (inputSampleL > Float(1))
        inputSampleL = Float(1);
    if (inputSampleL < -Float(1))
        inputSampleL = -Float(1);
    auto basscutL = 0.98;
    // we're going to be shifting this as the stages progress
    auto inputlevelL = startlevel;
    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleAL = (iirSampleAL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleAL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    auto bridgerectifier = (smoothAL + inputSampleL);
    smoothAL             = inputSampleL;
    inputSampleL         = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleBL = (iirSampleBL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleBL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothBL + inputSampleL);
    smoothBL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * fixB[fix_a0]) + fixB[fix_sL1];
    fixB[fix_sL1] = (inputSampleL * fixB[fix_a1]) - (outSample * fixB[fix_b1]) + fixB[fix_sL2];
    fixB[fix_sL2] = (inputSampleL * fixB[fix_a2]) - (outSample * fixB[fix_b2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleCL = (iirSampleCL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleCL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothCL + inputSampleL);
    smoothCL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleDL = (iirSampleDL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleDL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothDL + inputSampleL);
    smoothDL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * fixC[fix_a0]) + fixC[fix_sL1];
    fixC[fix_sL1] = (inputSampleL * fixC[fix_a1]) - (outSample * fixC[fix_b1]) + fixC[fix_sL2];
    fixC[fix_sL2] = (inputSampleL * fixC[fix_a2]) - (outSample * fixC[fix_b2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleEL = (iirSampleEL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleEL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothEL + inputSampleL);
    smoothEL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleFL = (iirSampleFL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleFL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothFL + inputSampleL);
    smoothFL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * fixD[fix_a0]) + fixD[fix_sL1];
    fixD[fix_sL1] = (inputSampleL * fixD[fix_a1]) - (outSample * fixD[fix_b1]) + fixD[fix_sL2];
    fixD[fix_sL2] = (inputSampleL * fixD[fix_a2]) - (outSample * fixD[fix_b2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleGL = (iirSampleGL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleGL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothGL + inputSampleL);
    smoothGL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleHL = (iirSampleHL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleHL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothHL + inputSampleL);
    smoothHL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * fixE[fix_a0]) + fixE[fix_sL1];
    fixE[fix_sL1] = (inputSampleL * fixE[fix_a1]) - (outSample * fixE[fix_b1]) + fixE[fix_sL2];
    fixE[fix_sL2] = (inputSampleL * fixE[fix_a2]) - (outSample * fixE[fix_b2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleIL = (iirSampleIL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleIL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothIL + inputSampleL);
    smoothIL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleJL = (iirSampleJL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleJL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothJL + inputSampleL);
    smoothJL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    outSample     = (inputSampleL * fixF[fix_a0]) + fixF[fix_sL1];
    fixF[fix_sL1] = (inputSampleL * fixF[fix_a1]) - (outSample * fixF[fix_b1]) + fixF[fix_sL2];
    fixF[fix_sL2] = (inputSampleL * fixF[fix_a2]) - (outSample * fixF[fix_b2]);
    inputSampleL  = outSample;  // fixed biquad filtering ultrasonics

    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleKL = (iirSampleKL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleKL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothKL + inputSampleL);
    smoothKL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass
    inputSampleL *= inputlevelL;
    inputlevelL = ((inputlevelL * Float(7)) + Float(1)) / Float(8);
    iirSampleLL = (iirSampleLL * (Float(1) - EQ)) + (inputSampleL * EQ);
    basscutL *= bassfactor;
    inputSampleL = inputSampleL - (iirSampleLL * basscutL);
    // highpass
    inputSampleL -= (inputSampleL * (etl::abs(inputSampleL) * Float(0.654)) * (etl::abs(inputSampleL) * Float(0.654)));
    // overdrive
    bridgerectifier = (smoothLL + inputSampleL);
    smoothLL        = inputSampleL;
    inputSampleL    = bridgerectifier;
    // two-sample averaging lowpass

    iirLowpassL  = (iirLowpassL * (Float(1) - toneEQ)) + (inputSampleL * toneEQ);
    inputSampleL = iirLowpassL;
    // lowpass. The only one of this type.
    // lowpass. The only one of this type.

    iirSpkAL = (iirSpkAL * (Float(1) - BEQ)) + (inputSampleL * BEQ);
    // extra lowpass for 4*12" speakers
    // extra lowpass for 4*12" speakers

    if (count < 0 || count > 128) {
        count = 128;
    }
    auto resultBL = 0.0;
    if (flip) {
        OddL[count + 128] = OddL[count] = iirSpkAL;
        resultBL                        = (OddL[count + down] + OddL[count + side] + OddL[count + diagonal]);
    } else {
        EvenL[count + 128] = EvenL[count] = iirSpkAL;
        resultBL                          = (EvenL[count + down] + EvenL[count + side] + EvenL[count + diagonal]);
    }
    count--;
    iirSpkBL = (iirSpkBL * (Float(1) - BEQ)) + (resultBL * BEQ);
    inputSampleL += (iirSpkBL * bleed);
    // extra lowpass for 4*12" speakers
    // extra lowpass for 4*12" speakers

    bridgerectifier = etl::abs(inputSampleL * outputlevel);
    if (bridgerectifier > Float(1.57079633))
        bridgerectifier = Float(1.57079633);
    bridgerectifier = etl::sin(bridgerectifier);
    if (inputSampleL > 0)
        inputSampleL = bridgerectifier;
    else
        inputSampleL = -bridgerectifier;

    if (bridgerectifier > Float(1.57079633))
        bridgerectifier = Float(1.57079633);
    bridgerectifier = etl::sin(bridgerectifier);

    iirSubL = (iirSubL * (Float(1) - BEQ)) + (inputSampleL * BEQ);
    inputSampleL += (iirSubL * bassfill * outputlevel);

    auto randy   = (_dist(_rng) * 0.053);
    inputSampleL = ((inputSampleL * (Float(1) - randy)) + (storeSampleL * randy)) * outputlevel;
    storeSampleL = inputSampleL;

    randy = (_dist(_rng) * 0.053);

    flip = !flip;

    if (wet != Float(1)) {
        inputSampleL = (inputSampleL * wet) + (drySampleL * (Float(1) - wet));
    }
    // Dry/Wet control, defaults to the last slider
    // amp

    cycle++;
    if (cycle == cycleEnd) {
        auto temp    = (inputSampleL + smoothCabAL) / Float(3);
        smoothCabAL  = inputSampleL;
        inputSampleL = temp;

        bL[84] = bL[83];
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
        bL[0]  = inputSampleL;
        inputSampleL += (bL[1] * (Float(1.31698250313308396) - (Float(0.08140616497621633) * etl::abs(bL[1]))));
        inputSampleL += (bL[2] * (Float(1.47229016949915326) - (Float(0.27680278993637253) * etl::abs(bL[2]))));
        inputSampleL += (bL[3] * (Float(1.30410109086044956) - (Float(0.35629113432046489) * etl::abs(bL[3]))));
        inputSampleL += (bL[4] * (Float(0.81766210474551260) - (Float(0.26808782337659753) * etl::abs(bL[4]))));
        inputSampleL += (bL[5] * (Float(0.19868872545506663) - (Float(0.11105517193919669) * etl::abs(bL[5]))));
        inputSampleL -= (bL[6] * (Float(0.39115909132567039) - (Float(0.12630622002682679) * etl::abs(bL[6]))));
        inputSampleL -= (bL[7] * (Float(0.76881891559343574) - (Float(0.40879849500403143) * etl::abs(bL[7]))));
        inputSampleL -= (bL[8] * (Float(0.87146861782680340) - (Float(0.59529560488000599) * etl::abs(bL[8]))));
        inputSampleL -= (bL[9] * (Float(0.79504575932563670) - (Float(0.60877047551611796) * etl::abs(bL[9]))));
        inputSampleL -= (bL[10] * (Float(0.61653017622406314) - (Float(0.47662851438557335) * etl::abs(bL[10]))));
        inputSampleL -= (bL[11] * (Float(0.40718195794382067) - (Float(0.24955839378539713) * etl::abs(bL[11]))));
        inputSampleL -= (bL[12] * (Float(0.31794900040616203) - (Float(0.04169792259600613) * etl::abs(bL[12]))));
        inputSampleL -= (bL[13] * (Float(0.41075032540217843) + (Float(0.00368483996076280) * etl::abs(bL[13]))));
        inputSampleL -= (bL[14] * (Float(0.56901352922170667) - (Float(0.11027360805893105) * etl::abs(bL[14]))));
        inputSampleL -= (bL[15] * (Float(0.62443222391889264) - (Float(0.22198075154245228) * etl::abs(bL[15]))));
        inputSampleL -= (bL[16] * (Float(0.53462856723129204) - (Float(0.22933544545324852) * etl::abs(bL[16]))));
        inputSampleL -= (bL[17] * (Float(0.34441703361995046) - (Float(0.12956809502269492) * etl::abs(bL[17]))));
        inputSampleL -= (bL[18] * (Float(0.13947052337867882) + (Float(0.00339775055962799) * etl::abs(bL[18]))));
        inputSampleL += (bL[19] * (Float(0.03771252648928484) - (Float(0.10863931549251718) * etl::abs(bL[19]))));
        inputSampleL += (bL[20] * (Float(0.18280210770271693) - (Float(0.17413646599296417) * etl::abs(bL[20]))));
        inputSampleL += (bL[21] * (Float(0.24621986701761467) - (Float(0.14547053270435095) * etl::abs(bL[21]))));
        inputSampleL += (bL[22] * (Float(0.22347075142737360) - (Float(0.02493869490104031) * etl::abs(bL[22]))));
        inputSampleL += (bL[23] * (Float(0.14346348482123716) + (Float(0.11284054747963246) * etl::abs(bL[23]))));
        inputSampleL += (bL[24] * (Float(0.00834364862916028) + (Float(0.24284684053733926) * etl::abs(bL[24]))));
        inputSampleL -= (bL[25] * (Float(0.11559740296078347) - (Float(0.32623054435304538) * etl::abs(bL[25]))));
        inputSampleL -= (bL[26] * (Float(0.18067604561283060) - (Float(0.32311481551122478) * etl::abs(bL[26]))));
        inputSampleL -= (bL[27] * (Float(0.22927997789035612) - (Float(0.26991539052832925) * etl::abs(bL[27]))));
        inputSampleL -= (bL[28] * (Float(0.28487666578669446) - (Float(0.22437227250279349) * etl::abs(bL[28]))));
        inputSampleL -= (bL[29] * (Float(0.31992973037153838) - (Float(0.15289876100963865) * etl::abs(bL[29]))));
        inputSampleL -= (bL[30] * (Float(0.35174606303520733) - (Float(0.05656293023086628) * etl::abs(bL[30]))));
        inputSampleL -= (bL[31] * (Float(0.36894898011375254) + (Float(0.04333925421463558) * etl::abs(bL[31]))));
        inputSampleL -= (bL[32] * (Float(0.32567576055307507) + (Float(0.14594589410921388) * etl::abs(bL[32]))));
        inputSampleL -= (bL[33] * (Float(0.27440135050585784) + (Float(0.15529667398122521) * etl::abs(bL[33]))));
        inputSampleL -= (bL[34] * (Float(0.21998973785078091) + (Float(0.05083553737157104) * etl::abs(bL[34]))));
        inputSampleL -= (bL[35] * (Float(0.10323624876862457) - (Float(0.04651829594199963) * etl::abs(bL[35]))));
        inputSampleL += (bL[36] * (Float(0.02091603687851074) + (Float(0.12000046818439322) * etl::abs(bL[36]))));
        inputSampleL += (bL[37] * (Float(0.11344930914138468) + (Float(0.17697142512225839) * etl::abs(bL[37]))));
        inputSampleL += (bL[38] * (Float(0.22766779627643968) + (Float(0.13645102964003858) * etl::abs(bL[38]))));
        inputSampleL += (bL[39] * (Float(0.38378309953638229) - (Float(0.01997653307333791) * etl::abs(bL[39]))));
        inputSampleL += (bL[40] * (Float(0.52789400804568076) - (Float(0.21409137428422448) * etl::abs(bL[40]))));
        inputSampleL += (bL[41] * (Float(0.55444630296938280) - (Float(0.32331980931576626) * etl::abs(bL[41]))));
        inputSampleL += (bL[42] * (Float(0.42333237669264601) - (Float(0.26855847463044280) * etl::abs(bL[42]))));
        inputSampleL += (bL[43] * (Float(0.21942831522035078) - (Float(0.12051365248820624) * etl::abs(bL[43]))));
        inputSampleL -= (bL[44] * (Float(0.00584169427830633) - (Float(0.03706970171280329) * etl::abs(bL[44]))));
        inputSampleL -= (bL[45] * (Float(0.24279799124660351) - (Float(0.17296440491477982) * etl::abs(bL[45]))));
        inputSampleL -= (bL[46] * (Float(0.40173760787507085) - (Float(0.21717989835163351) * etl::abs(bL[46]))));
        inputSampleL -= (bL[47] * (Float(0.43930035724188155) - (Float(0.16425928481378199) * etl::abs(bL[47]))));
        inputSampleL -= (bL[48] * (Float(0.41067765934041811) - (Float(0.10390115786636855) * etl::abs(bL[48]))));
        inputSampleL -= (bL[49] * (Float(0.34409235547165967) - (Float(0.07268159377411920) * etl::abs(bL[49]))));
        inputSampleL -= (bL[50] * (Float(0.26542883122568151) - (Float(0.05483457497365785) * etl::abs(bL[50]))));
        inputSampleL -= (bL[51] * (Float(0.22024754776138800) - (Float(0.06484897950087598) * etl::abs(bL[51]))));
        inputSampleL -= (bL[52] * (Float(0.20394367993632415) - (Float(0.08746309731952180) * etl::abs(bL[52]))));
        inputSampleL -= (bL[53] * (Float(0.17565242431124092) - (Float(0.07611309538078760) * etl::abs(bL[53]))));
        inputSampleL -= (bL[54] * (Float(0.10116623231246825) - (Float(0.00642818706295112) * etl::abs(bL[54]))));
        inputSampleL -= (bL[55] * (Float(0.00782648272053632) + (Float(0.08004141267685004) * etl::abs(bL[55]))));
        inputSampleL += (bL[56] * (Float(0.05059046006747323) - (Float(0.12436676387548490) * etl::abs(bL[56]))));
        inputSampleL += (bL[57] * (Float(0.06241531553254467) - (Float(0.11530779547021434) * etl::abs(bL[57]))));
        inputSampleL += (bL[58] * (Float(0.04952694587101836) - (Float(0.08340945324333944) * etl::abs(bL[58]))));
        inputSampleL += (bL[59] * (Float(0.00843873294401687) - (Float(0.03279659052562903) * etl::abs(bL[59]))));
        inputSampleL -= (bL[60] * (Float(0.05161338949440241) - (Float(0.03428181149163798) * etl::abs(bL[60]))));
        inputSampleL -= (bL[61] * (Float(0.08165520146902012) - (Float(0.08196746092283110) * etl::abs(bL[61]))));
        inputSampleL -= (bL[62] * (Float(0.06639532849935320) - (Float(0.09797462781896329) * etl::abs(bL[62]))));
        inputSampleL -= (bL[63] * (Float(0.02953430910661621) - (Float(0.09175612938515763) * etl::abs(bL[63]))));
        inputSampleL += (bL[64] * (Float(0.00741058547442938) + (Float(0.05442091048731967) * etl::abs(bL[64]))));
        inputSampleL += (bL[65] * (Float(0.01832866125391727) + (Float(0.00306243693643687) * etl::abs(bL[65]))));
        inputSampleL += (bL[66] * (Float(0.00526964230373573) - (Float(0.04364102661136410) * etl::abs(bL[66]))));
        inputSampleL -= (bL[67] * (Float(0.00300984373848200) + (Float(0.09742737841278880) * etl::abs(bL[67]))));
        inputSampleL -= (bL[68] * (Float(0.00413616769576694) + (Float(0.14380661694523073) * etl::abs(bL[68]))));
        inputSampleL -= (bL[69] * (Float(0.00588769034931419) + (Float(0.16012843578892538) * etl::abs(bL[69]))));
        inputSampleL -= (bL[70] * (Float(0.00688588239450581) + (Float(0.14074464279305798) * etl::abs(bL[70]))));
        inputSampleL -= (bL[71] * (Float(0.02277307992926315) + (Float(0.07914752191801366) * etl::abs(bL[71]))));
        inputSampleL -= (bL[72] * (Float(0.04627166091180877) - (Float(0.00192787268067208) * etl::abs(bL[72]))));
        inputSampleL -= (bL[73] * (Float(0.05562045897455786) - (Float(0.05932868727665747) * etl::abs(bL[73]))));
        inputSampleL -= (bL[74] * (Float(0.05134243784922165) - (Float(0.08245334798868090) * etl::abs(bL[74]))));
        inputSampleL -= (bL[75] * (Float(0.04719409472239919) - (Float(0.07498680629253825) * etl::abs(bL[75]))));
        inputSampleL -= (bL[76] * (Float(0.05889738914266415) - (Float(0.06116127018043697) * etl::abs(bL[76]))));
        inputSampleL -= (bL[77] * (Float(0.09428363535111127) - (Float(0.06535868867863834) * etl::abs(bL[77]))));
        inputSampleL -= (bL[78] * (Float(0.15181756953225126) - (Float(0.08982979655234427) * etl::abs(bL[78]))));
        inputSampleL -= (bL[79] * (Float(0.20878969456036670) - (Float(0.10761070891499538) * etl::abs(bL[79]))));
        inputSampleL -= (bL[80] * (Float(0.22647885581813790) - (Float(0.08462542510349125) * etl::abs(bL[80]))));
        inputSampleL -= (bL[81] * (Float(0.19723482443646323) - (Float(0.02665160920736287) * etl::abs(bL[81]))));
        inputSampleL -= (bL[82] * (Float(0.16441643451155163) + (Float(0.02314691954338197) * etl::abs(bL[82]))));
        inputSampleL -= (bL[83] * (Float(0.15201914054931515) + (Float(0.04424903493886839) * etl::abs(bL[83]))));
        inputSampleL -= (bL[84] * (Float(0.15454370641307855) + (Float(0.04223203797913008) * etl::abs(bL[84]))));

        temp         = (inputSampleL + smoothCabBL) / Float(3);
        smoothCabBL  = inputSampleL;
        inputSampleL = temp / 4.0;

        randy = (_dist(_rng) * 0.057);
        drySampleL
            = ((((inputSampleL * (1 - randy)) + (lastCabSampleL * randy)) * wet) + (drySampleL * (Float(1) - wet)))
            * outputlevel;
        lastCabSampleL = inputSampleL;
        inputSampleL   = drySampleL;  // cab L

        if (cycleEnd == 4) {
            lastRefL[0] = lastRefL[4];                       // start from previous last
            lastRefL[2] = (lastRefL[0] + inputSampleL) / 2;  // half
            lastRefL[1] = (lastRefL[0] + lastRefL[2]) / 2;   // one quarter
            lastRefL[3] = (lastRefL[2] + inputSampleL) / 2;  // three quarters
            lastRefL[4] = inputSampleL;                      // full
        }
        if (cycleEnd == 3) {
            lastRefL[0] = lastRefL[3];                                      // start from previous last
            lastRefL[2] = (lastRefL[0] + lastRefL[0] + inputSampleL) / 3;   // third
            lastRefL[1] = (lastRefL[0] + inputSampleL + inputSampleL) / 3;  // two thirds
            lastRefL[3] = inputSampleL;                                     // full
        }
        if (cycleEnd == 2) {
            lastRefL[0] = lastRefL[2];                       // start from previous last
            lastRefL[1] = (lastRefL[0] + inputSampleL) / 2;  // half
            lastRefL[2] = inputSampleL;                      // full
        }
        if (cycleEnd == 1) {
            lastRefL[0] = inputSampleL;
        }
        cycle        = 0;  // reset
        inputSampleL = lastRefL[cycle];
    } else {
        inputSampleL = lastRefL[cycle];
        // we are going through our references now
    }
    switch (cycleEnd)  // multi-pole average using lastRef[] variables
    {
        case 4:
            lastRefL[8]  = inputSampleL;
            inputSampleL = (inputSampleL + lastRefL[7]) * 0.5;
            lastRefL[7]  = lastRefL[8];  // continue, do not break
            [[fallthrough]];
        case 3:
            lastRefL[8]  = inputSampleL;
            inputSampleL = (inputSampleL + lastRefL[6]) * 0.5;
            lastRefL[6]  = lastRefL[8];  // continue, do not break
            [[fallthrough]];
        case 2:
            lastRefL[8]  = inputSampleL;
            inputSampleL = (inputSampleL + lastRefL[5]) * 0.5;
            lastRefL[5]  = lastRefL[8];  // continue, do not break
        case 1: break;                   // no further averaging
    }

    return inputSampleL;
}

template<etl::floating_point Float, typename URNG>
auto AirWindowsFireAmp<Float, URNG>::reset() -> void
{
    lastSampleL  = 0.0;
    storeSampleL = 0.0;
    smoothAL     = 0.0;
    smoothBL     = 0.0;
    smoothCL     = 0.0;
    smoothDL     = 0.0;
    smoothEL     = 0.0;
    smoothFL     = 0.0;
    smoothGL     = 0.0;
    smoothHL     = 0.0;
    smoothIL     = 0.0;
    smoothJL     = 0.0;
    smoothKL     = 0.0;
    smoothLL     = 0.0;
    iirSampleAL  = 0.0;
    iirSampleBL  = 0.0;
    iirSampleCL  = 0.0;
    iirSampleDL  = 0.0;
    iirSampleEL  = 0.0;
    iirSampleFL  = 0.0;
    iirSampleGL  = 0.0;
    iirSampleHL  = 0.0;
    iirSampleIL  = 0.0;
    iirSampleJL  = 0.0;
    iirSampleKL  = 0.0;
    iirSampleLL  = 0.0;
    iirLowpassL  = 0.0;
    iirSpkAL     = 0.0;
    iirSpkBL     = 0.0;
    iirSubL      = 0.0;

    for (int fcount = 0; fcount < 257; fcount++) {
        OddL[fcount]  = 0.0;
        EvenL[fcount] = 0.0;
    }

    count = 0;
    flip  = false;  // amp

    for (int fcount = 0; fcount < 90; fcount++) {
        bL[fcount] = 0;
    }
    smoothCabAL    = 0.0;
    smoothCabBL    = 0.0;
    lastCabSampleL = 0.0;  // cab

    for (int fcount = 0; fcount < 9; fcount++) {
        lastRefL[fcount] = 0.0;
    }
    cycle = 0;  // undersampling

    for (int x = 0; x < fix_total; x++) {
        fixA[x] = 0.0;
        fixB[x] = 0.0;
        fixC[x] = 0.0;
        fixD[x] = 0.0;
        fixE[x] = 0.0;
        fixF[x] = 0.0;
    }  // filtering
}

}  // namespace grit
