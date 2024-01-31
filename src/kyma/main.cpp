#include <grit/audio/delay/static_delay_line.hpp>
#include <grit/audio/envelope/adsr.hpp>
#include <grit/audio/music/note.hpp>
#include <grit/audio/oscillator/variable_shape_oscillator.hpp>
#include <grit/audio/oscillator/wavetable_oscillator.hpp>
#include <grit/audio/unit/decibel.hpp>
#include <grit/math/dynamic_smoothing.hpp>
#include <grit/math/range.hpp>

#include <daisy_patch_sm.h>

namespace kyma {
static constexpr auto block_size     = 16U;
static constexpr auto sample_rate    = 96'000.0F;
static constexpr auto wavetable_sine = grit::makeSineWavetable<float, 2048>();
static constexpr auto sine_wavetable = etl::mdspan{
    wavetable_sine.data(),
    etl::extents<etl::size_t, wavetable_sine.size()>{},
};

auto sub_octave_toggle  = daisy::Switch{};
auto env_trigger_button = daisy::Switch{};
auto patch              = daisy::patch_sm::DaisyPatchSM{};
auto& envelope_gate     = patch.gate_in_1;

auto adsr           = grit::adsr{};
auto oscillator     = grit::wavetable_oscillator<float, wavetable_sine.size()>{sine_wavetable};
auto sub_oscillator = grit::wavetable_oscillator<float, wavetable_sine.size()>{sine_wavetable};

// auto smoothE = grit::DynamicSmoothing<float, grit::DynamicSmoothingType::Efficient>{};
// auto smoothA = grit::DynamicSmoothing<float, grit::DynamicSmoothingType::Accurate>{};
// auto delayN  = grit::StaticDelayLine<float, 32, grit::BufferInterpolation::None>{};
// auto delayL  = grit::StaticDelayLine<float, 32, grit::BufferInterpolation::Linear>{};
// auto delayH  = grit::StaticDelayLine<float, 32, grit::BufferInterpolation::Hermite>{};

auto audio_callback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    patch.ProcessAllControls();

    auto const pitch_knob   = patch.GetAdcValue(daisy::patch_sm::CV_1);
    auto const attack_knob  = patch.GetAdcValue(daisy::patch_sm::CV_2);
    auto const morph_knob   = patch.GetAdcValue(daisy::patch_sm::CV_3);
    auto const release_knob = patch.GetAdcValue(daisy::patch_sm::CV_4);

    auto const v_oct_cv     = patch.GetAdcValue(daisy::patch_sm::CV_5);
    auto const morph_cv     = patch.GetAdcValue(daisy::patch_sm::CV_6);
    auto const sub_gain_cv  = patch.GetAdcValue(daisy::patch_sm::CV_7);
    auto const sub_morph_cv = patch.GetAdcValue(daisy::patch_sm::CV_8);

    auto const pitch            = grit::mapToRange(pitch_knob, 36.0F, 96.0F);
    auto const volts_per_octave = grit::mapToRange(v_oct_cv, 0.0F, 60.0F);
    auto const note             = etl::clamp(pitch + volts_per_octave, 0.0F, 127.0F);
    auto const morph            = etl::clamp(morph_knob + morph_cv, 0.0F, 1.0F);

    auto const sub_offset      = sub_octave_toggle.Pressed() ? 12.0F : 24.0F;
    auto const sub_note_number = etl::clamp(note - sub_offset, 0.0F, 127.0F);
    auto const sub_morph       = etl::clamp(sub_morph_cv, 0.0F, 1.0F);
    auto const sub_gain        = grit::mapToRange(sub_gain_cv, 0.0F, 1.0F);

    auto const attack  = grit::mapToRange(attack_knob, 0.0F, 0.750F);
    auto const release = grit::mapToRange(release_knob, 0.0F, 2.5F);

    // oscillator.setWavetable(SineWavetable);
    // subOscillator.setWavetable(SineWavetable);
    // oscillator.setShapeMorph(morph);
    // subOscillator.setShapeMorph(subMorph);
    etl::ignore_unused(sub_morph, morph);

    oscillator.set_frequency(grit::noteToHertz(note));
    sub_oscillator.set_frequency(grit::noteToHertz(sub_note_number));

    adsr.set_attack(attack * sample_rate);
    adsr.set_release(release * sample_rate);
    adsr.gate(envelope_gate.State() or env_trigger_button.Pressed());

    for (size_t i = 0; i < size; ++i) {
        auto const fm_modulator = IN_L[i];
        auto const fm_amount    = IN_R[i];
        oscillator.add_phase_offset(fm_modulator * fm_amount);

        auto const env = adsr.process_sample();
        patch.WriteCvOut(daisy::patch_sm::CV_OUT_1, env * 5.0F);
        patch.WriteCvOut(daisy::patch_sm::CV_OUT_2, env * 5.0F);

        auto const osc = oscillator() * env;
        auto const sub = sub_oscillator() * env * sub_gain;

        OUT_L[i] = sub * 0.75F;
        OUT_R[i] = osc * 0.75F;
    }
}

}  // namespace kyma

auto main() -> int
{
    using namespace kyma;

    patch.Init();
    patch.SetAudioSampleRate(sample_rate);
    patch.SetAudioBlockSize(block_size);
    patch.StartAudio(audio_callback);

    sub_octave_toggle.Init(daisy::patch_sm::DaisyPatchSM::B8);
    env_trigger_button.Init(daisy::patch_sm::DaisyPatchSM::B7);

    oscillator.set_sample_rate(sample_rate);
    sub_oscillator.set_sample_rate(sample_rate);

    while (true) {
        sub_octave_toggle.Debounce();
        env_trigger_button.Debounce();
        patch.SetLed(not sub_octave_toggle.Pressed());
    }
}
