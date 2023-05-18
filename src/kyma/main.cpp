#include "mc/dsp/adsr.hpp"
#include "mc/dsp/note.hpp"
#include "mc/dsp/oscillator.hpp"
#include "mc/dsp/range.hpp"

#include "daisy_patch_sm.h"

static constexpr auto BLOCK_SIZE  = 4U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto adsr       = mc::ADSR{};
auto oscillator = mc::Oscillator<float>{};
auto patch      = daisy::patch_sm::DaisyPatchSM{};

auto audioCallback(daisy::AudioHandle::InputBuffer /*in*/, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    auto const coarseKnob = patch.GetAdcValue(daisy::patch_sm::CV_1);
    auto const coarse     = mc::mapToLinearRange(coarseKnob, 36.0F, 96.0F);

    auto const voctCV = patch.GetAdcValue(daisy::patch_sm::CV_5);
    auto const voct   = mc::mapToLinearRange(voctCV, 0.0F, 60.0F);

    auto const noteNumber = mc::clamp(coarse + voct, 0.0F, 127.0F);
    auto const freq       = mc::noteToFrequency(noteNumber);

    oscillator.setFrequency(freq);
    adsr.setAttack(0.025F * SAMPLE_RATE);
    adsr.setRelease(1.5F * SAMPLE_RATE);

    for (size_t i = 0; i < size; ++i)
    {
        auto* const leftOut  = out[0];
        auto* const rightOut = out[1];

        auto const sample = oscillator();
        leftOut[i]        = sample;
        rightOut[i]       = sample;
    }
}

auto main() -> int
{
    oscillator.setShape(mc::OscillatorShape::Sine);
    oscillator.setSampleRate(SAMPLE_RATE);

    patch.Init();
    patch.SetAudioSampleRate(SAMPLE_RATE);
    patch.SetAudioBlockSize(BLOCK_SIZE);
    patch.StartAudio(audioCallback);

    while (true) {}
}
