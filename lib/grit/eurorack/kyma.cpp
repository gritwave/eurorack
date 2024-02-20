#include "kyma.hpp"

#include <grit/audio/music/note.hpp>
#include <grit/math/remap.hpp>
#include <grit/unit/decibel.hpp>

namespace grit {

auto Kyma::prepare(float sampleRate, etl::size_t blockSize) -> void
{
    _sampleRate = sampleRate;
    oscillator.setSampleRate(sampleRate);
    subOscillator.setSampleRate(sampleRate);

    auto const blockRate = sampleRate / static_cast<float>(blockSize);
    _pitchKnob.setSampleRate(blockRate);
    _morphKnob.setSampleRate(blockRate);
    _attackKnob.setSampleRate(blockRate);
    _releaseKnob.setSampleRate(blockRate);
    _vOctCV.setSampleRate(blockRate);
    _morphCV.setSampleRate(blockRate);
    _subGainCV.setSampleRate(blockRate);
    _subMorphCV.setSampleRate(blockRate);
}

auto Kyma::process(StereoBlock<float> const& buffer, ControlInput const& inputs) -> float
{
    auto const pitchKnob   = _pitchKnob(inputs.pitchKnob);
    auto const attackKnob  = _morphKnob(inputs.morphKnob);
    auto const morphKnob   = _attackKnob(inputs.attackKnob);
    auto const releaseKnob = _releaseKnob(inputs.releaseKnob);

    auto const vOctCv     = _vOctCV(inputs.vOctCV);
    auto const morphCv    = _morphCV(inputs.morphCV);
    auto const subGainCv  = _subGainCV(inputs.subGainCV);
    auto const subMorphCv = _subMorphCV(inputs.subMorphCV);

    auto const pitch          = grit::remap(pitchKnob, 36.0F, 96.0F);
    auto const voltsPerOctave = grit::remap(vOctCv, 0.0F, 60.0F);
    auto const note           = etl::clamp(pitch + voltsPerOctave, 0.0F, 127.0F);
    auto const morph          = etl::clamp(morphKnob + morphCv, 0.0F, 1.0F);

    auto const subOffset     = inputs.subShift ? 12.0F : 24.0F;
    auto const subNoteNumber = etl::clamp(note - subOffset, 0.0F, 127.0F);
    auto const subMorph      = etl::clamp(subMorphCv, 0.0F, 1.0F);
    auto const subGain       = grit::remap(subGainCv, 0.0F, 1.0F);

    auto const attack  = grit::remap(attackKnob, 0.0F, 0.750F);
    auto const release = grit::remap(releaseKnob, 0.0F, 2.5F);

    // oscillator.setWavetable(SineWavetable);
    // subOscillator.setWavetable(SineWavetable);
    // oscillator.setShapeMorph(morph);
    // subOscillator.setShapeMorph(subMorph);
    etl::ignore_unused(subMorph, morph);

    oscillator.setFrequency(grit::noteToHertz(note));
    subOscillator.setFrequency(grit::noteToHertz(subNoteNumber));

    adsr.setAttack(attack * _sampleRate);
    adsr.setRelease(release * _sampleRate);
    adsr.gate(inputs.gate);

    auto env = 0.0F;

    for (size_t i = 0; i < buffer.extent(1); ++i) {
        auto const fmModulator = buffer(0, i);
        auto const fmAmount    = buffer(1, i);
        oscillator.addPhaseOffset(fmModulator * fmAmount);
        env = adsr();

        auto const osc = oscillator() * env;
        auto const sub = subOscillator() * env * subGain;

        buffer(0, i) = sub * 0.75F;
        buffer(1, i) = osc * 0.75F;
    }

    return env;
}

}  // namespace grit
