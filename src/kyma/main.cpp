#include <digitaldreams/audio/envelope/adsr.hpp>
#include <digitaldreams/audio/math/decibel.hpp>
#include <digitaldreams/audio/math/range.hpp>
#include <digitaldreams/audio/music/note.hpp>
#include <digitaldreams/audio/oscillator/variable_shape_oscillator.hpp>

#include <daisy_patch_sm.h>

namespace audio = digitaldreams::audio;

static constexpr auto BLOCK_SIZE  = 16U;
static constexpr auto SAMPLE_RATE = 96'000.0F;

auto subOctaveToggle  = daisy::Switch{};
auto envTriggerButton = daisy::Switch{};
auto patch            = daisy::patch_sm::DaisyPatchSM{};
auto& envelopeGate    = patch.gate_in_1;
auto lastEnvelopeGate = false;

auto adsr          = audio::ADSR{};
auto oscillator    = audio::VariableShapeOscillator<float>{};
auto subOscillator = audio::VariableShapeOscillator<float>{};

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

    auto const pitch          = audio::mapToRange(pitchKnob, 36.0F, 96.0F);
    auto const voltsPerOctave = audio::mapToRange(vOctCV, 0.0F, 60.0F);
    auto const note           = etl::clamp(pitch + voltsPerOctave, 0.0F, 127.0F);
    auto const morph          = etl::clamp(morphKnob + morphCV, 0.0F, 1.0F);

    auto const subOffset     = subOctaveToggle.Pressed() ? 12.0F : 24.0F;
    auto const subNoteNumber = etl::clamp(note - subOffset, 0.0F, 127.0F);
    auto const subMorph      = etl::clamp(subMorphCV, 0.0F, 1.0F);
    auto const subGain       = audio::mapToRange(subGainCV, 0.0F, 1.0F);

    auto const attack  = audio::mapToRange(attackKnob, 0.0F, 0.750F);
    auto const release = audio::mapToRange(releaseKnob, 0.0F, 2.5F);

    oscillator.setFrequency(audio::noteToHertz(note));
    oscillator.setShapeMorph(morph);

    subOscillator.setFrequency(audio::noteToHertz(subNoteNumber));
    subOscillator.setShapeMorph(subMorph);

    adsr.setAttack(attack * SAMPLE_RATE);
    adsr.setRelease(release * SAMPLE_RATE);
    adsr.gate(envelopeGate.State() or envTriggerButton.Pressed());

    for (size_t i = 0; i < size; ++i)
    {
        auto const fmModulator = IN_L[i];
        auto const fmAmount    = IN_R[i];
        oscillator.addPhaseOffset(fmModulator * fmAmount);

        auto const env = adsr.processSample();
        patch.WriteCvOut(daisy::patch_sm::CV_OUT_1, env * 5.0F);
        patch.WriteCvOut(daisy::patch_sm::CV_OUT_2, env * 5.0F);

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
    envTriggerButton.Init(patch.B7);

    oscillator.setShapes(audio::OscillatorShape::Sine, audio::OscillatorShape::Square);
    oscillator.setSampleRate(SAMPLE_RATE);

    subOscillator.setShapes(audio::OscillatorShape::Sine, audio::OscillatorShape::Triangle);
    subOscillator.setSampleRate(SAMPLE_RATE);

    while (true)
    {
        subOctaveToggle.Debounce();
        envTriggerButton.Debounce();
        patch.SetLed(not subOctaveToggle.Pressed());
    }
}
