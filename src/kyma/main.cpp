#include "mc/envelope/adsr.hpp"
#include "mc/math/decibel.hpp"
#include "mc/math/range.hpp"
#include "mc/music/note.hpp"
#include "mc/oscillator/variable_shape_oscillator.hpp"

#include "daisy_patch_sm.h"

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto subOctaveToggle  = daisy::Switch{};
auto patch            = daisy::patch_sm::DaisyPatchSM{};
auto& envelopeGate    = patch.gate_in_1;
auto lastEnvelopeGate = false;

auto adsr          = mc::ADSR{};
auto oscillator    = mc::VariableShapeOscillator<float>{};
auto subOscillator = mc::VariableShapeOscillator<float>{};

auto audioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    auto const pitchKnob   = patch.GetAdcValue(daisy::patch_sm::CV_1);
    auto const attackKnob  = patch.GetAdcValue(daisy::patch_sm::CV_2);
    auto const morphKnob   = patch.GetAdcValue(daisy::patch_sm::CV_3);
    auto const releaseKnob = patch.GetAdcValue(daisy::patch_sm::CV_4);

    auto const vOctCV     = patch.GetAdcValue(daisy::patch_sm::CV_5);
    auto const morphCV    = patch.GetAdcValue(daisy::patch_sm::CV_6);
    auto const subGainCV  = patch.GetAdcValue(daisy::patch_sm::CV_7);
    auto const subMorphCV = patch.GetAdcValue(daisy::patch_sm::CV_8);

    auto const pitch          = mc::mapToRange(pitchKnob, 36.0F, 96.0F);
    auto const voltsPerOctave = mc::mapToRange(vOctCV, 0.0F, 60.0F);
    auto const note           = mc::clamp(pitch + voltsPerOctave, 0.0F, 127.0F);
    auto const morph          = mc::clamp(morphKnob + morphCV, 0.0F, 1.0F);

    auto const subOffset     = subOctaveToggle.Pressed() ? 24.0F : 12.0F;
    auto const subNoteNumber = mc::clamp(note - subOffset, 0.0F, 127.0F);
    auto const subMorph      = mc::clamp(subMorphCV, 0.0F, 1.0F);
    auto const subGain       = mc::mapToRange(subGainCV, 0.0F, 1.0F);

    auto const attack  = mc::mapToRange(attackKnob, 0.0F, 500.0F);
    auto const release = mc::mapToRange(releaseKnob, 0.0F, 500.0F);

    oscillator.setFrequency(mc::noteToHertz(note));
    oscillator.setShapeMorph(morph);

    subOscillator.setFrequency(mc::noteToHertz(subNoteNumber));
    subOscillator.setShapeMorph(subMorph);

    adsr.setAttack(attack * SAMPLE_RATE);
    adsr.setRelease(release * SAMPLE_RATE);
    adsr.gate(envelopeGate.State());

    for (size_t i = 0; i < size; ++i)
    {
        auto const fmModulator = IN_L[i];
        auto const fmAmount    = IN_R[i];
        oscillator.addPhaseOffset(fmModulator * fmAmount);

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

    oscillator.setShapes(mc::OscillatorShape::Sine, mc::OscillatorShape::Square);
    oscillator.setSampleRate(SAMPLE_RATE);

    subOscillator.setShapes(mc::OscillatorShape::Sine, mc::OscillatorShape::Triangle);
    subOscillator.setSampleRate(SAMPLE_RATE);

    while (true)
    {
        subOctaveToggle.Debounce();
        auto const state = subOctaveToggle.Pressed();
        patch.SetLed(state);
    }
}
