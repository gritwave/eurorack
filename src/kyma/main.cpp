#include <mc/audio/delay/static_delay_line.hpp>
#include <mc/audio/envelope/adsr.hpp>
#include <mc/audio/music/note.hpp>
#include <mc/audio/oscillator/variable_shape_oscillator.hpp>
#include <mc/audio/oscillator/wavetable_oscillator.hpp>
#include <mc/audio/unit/decibel.hpp>
#include <mc/math/dynamic_smoothing.hpp>
#include <mc/math/range.hpp>

#include <daisy_patch_sm.h>

namespace kyma
{
static constexpr auto BLOCK_SIZE     = 16U;
static constexpr auto SAMPLE_RATE    = 96'000.0F;
static constexpr auto WAVETABLE_SINE = mc::audio::makeSineWavetable<float, 2048>();

auto subOctaveToggle  = daisy::Switch{};
auto envTriggerButton = daisy::Switch{};
auto patch            = daisy::patch_sm::DaisyPatchSM{};
auto& envelopeGate    = patch.gate_in_1;

auto adsr          = mc::audio::ADSR{};
auto oscillator    = mc::audio::WavetableOscillator<float, WAVETABLE_SINE.size()>{WAVETABLE_SINE};
auto subOscillator = mc::audio::WavetableOscillator<float, WAVETABLE_SINE.size()>{WAVETABLE_SINE};

// auto smoothE = mc::DynamicSmoothing<float, mc::DynamicSmoothingType::Efficient>{};
// auto smoothA = mc::DynamicSmoothing<float, mc::DynamicSmoothingType::Accurate>{};
// auto delayN  = mc::audio::StaticDelayLine<float, 32, mc::audio::BufferInterpolation::None>{};
// auto delayL  = mc::audio::StaticDelayLine<float, 32, mc::audio::BufferInterpolation::Linear>{};
// auto delayH  = mc::audio::StaticDelayLine<float, 32, mc::audio::BufferInterpolation::Hermite>{};

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
    auto const note           = etl::clamp(pitch + voltsPerOctave, 0.0F, 127.0F);
    auto const morph          = etl::clamp(morphKnob + morphCV, 0.0F, 1.0F);

    auto const subOffset     = subOctaveToggle.Pressed() ? 12.0F : 24.0F;
    auto const subNoteNumber = etl::clamp(note - subOffset, 0.0F, 127.0F);
    auto const subMorph      = etl::clamp(subMorphCV, 0.0F, 1.0F);
    auto const subGain       = mc::mapToRange(subGainCV, 0.0F, 1.0F);

    auto const attack  = mc::mapToRange(attackKnob, 0.0F, 0.750F);
    auto const release = mc::mapToRange(releaseKnob, 0.0F, 2.5F);

    oscillator.setWavetable(WAVETABLE_SINE);
    subOscillator.setWavetable(WAVETABLE_SINE);
    // oscillator.setShapeMorph(morph);
    // subOscillator.setShapeMorph(subMorph);
    etl::ignore_unused(subMorph, morph);

    oscillator.setFrequency(mc::audio::noteToHertz(note));
    subOscillator.setFrequency(mc::audio::noteToHertz(subNoteNumber));

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
        auto const sub = subOscillator() * env;  //* subGain;
        etl::ignore_unused(subGain);

        OUT_L[i] = sub * 0.25F;
        OUT_R[i] = osc * 0.25F;
    }
}
}  // namespace kyma

auto main() -> int
{
    using namespace kyma;

    patch.Init();
    patch.SetAudioSampleRate(SAMPLE_RATE);
    patch.SetAudioBlockSize(BLOCK_SIZE);
    patch.StartAudio(audioCallback);

    subOctaveToggle.Init(patch.B8);
    envTriggerButton.Init(patch.B7);

    oscillator.setSampleRate(SAMPLE_RATE);
    subOscillator.setSampleRate(SAMPLE_RATE);

    while (true)
    {
        subOctaveToggle.Debounce();
        envTriggerButton.Debounce();
        patch.SetLed(not subOctaveToggle.Pressed());
    }
}
