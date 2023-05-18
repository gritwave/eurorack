#include "mc/dsp/adsr.hpp"
#include "mc/dsp/note.hpp"
#include "mc/dsp/oscillator.hpp"
#include "mc/dsp/range.hpp"

#include "daisy_patch_sm.h"

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto subOctaveToggle  = daisy::Switch{};
auto patch            = daisy::patch_sm::DaisyPatchSM{};
auto& envelopeGate    = patch.gate_in_1;
auto lastEnvelopeGate = false;

auto adsr          = mc::ADSR{};
auto oscillator    = mc::Oscillator<float>{};
auto subOscillator = mc::Oscillator<float>{};

auto audioCallback(daisy::AudioHandle::InputBuffer /*in*/, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    auto const coarseKnob = patch.GetAdcValue(daisy::patch_sm::CV_1);
    auto const coarse     = mc::mapToLinearRange(coarseKnob, 36.0F, 96.0F);

    auto const voctCV = patch.GetAdcValue(daisy::patch_sm::CV_5);
    auto const voct   = mc::mapToLinearRange(voctCV, 0.0F, 60.0F);

    auto const noteNumber = coarse + voct;
    oscillator.setFrequency(mc::noteToFrequency(mc::clamp(noteNumber, 0.0F, 127.0F)));

    auto const subOffset = subOctaveToggle.Pressed() ? 24.0F : 12.0F;
    subOscillator.setFrequency(mc::noteToFrequency(mc::clamp(noteNumber - subOffset, 0.0F, 127.0F)));

    auto const attackCV = patch.GetAdcValue(daisy::patch_sm::CV_2);
    auto const attack   = mc::mapToLinearRange(attackCV, 0.0F, 500.0F);
    adsr.setAttack(attack * SAMPLE_RATE);

    auto const releaseCV = patch.GetAdcValue(daisy::patch_sm::CV_4);
    auto const release   = mc::mapToLinearRange(releaseCV, 0.0F, 500.0F);
    adsr.setRelease(release * SAMPLE_RATE);

    auto const gate = envelopeGate.State();
    if (lastEnvelopeGate != gate)
    {
        adsr.gate();
        lastEnvelopeGate = gate;
    }

    auto const subGainCV = patch.GetAdcValue(daisy::patch_sm::CV_7);
    auto const subGain   = mc::mapToLinearRange(subGainCV, 0.0F, 1.0F);

    for (size_t i = 0; i < size; ++i)
    {
        auto const env = adsr.processSample();
        patch.WriteCvOut(daisy::patch_sm::CV_OUT_1, env * 5.0F);

        auto const osc = oscillator() * env;
        auto const sub = subOscillator() * env * subGain;

        OUT_L[i] = osc;
        OUT_R[i] = osc + sub;
    }
}

auto main() -> int
{
    patch.Init();
    patch.SetAudioSampleRate(SAMPLE_RATE);
    patch.SetAudioBlockSize(BLOCK_SIZE);
    patch.StartAudio(audioCallback);

    subOctaveToggle.Init(patch.B8);

    oscillator.setShape(mc::OscillatorShape::Square);
    oscillator.setSampleRate(SAMPLE_RATE);

    subOscillator.setShape(mc::OscillatorShape::Sine);
    subOscillator.setSampleRate(SAMPLE_RATE);

    while (true)
    {
        subOctaveToggle.Debounce();
        auto const state = subOctaveToggle.Pressed();
        patch.SetLed(state);
    }
}
