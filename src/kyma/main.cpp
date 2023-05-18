#include "mc/dsp/oscillator.hpp"

#include "daisy_patch_sm.h"

auto oscillator = mc::Oscillator<float>{};
auto patch      = daisy::patch_sm::DaisyPatchSM{};

static auto process(daisy::AudioHandle::InputBuffer /*in*/, daisy::AudioHandle::OutputBuffer out, size_t size) -> void
{
    auto* leftOut  = out[0];
    auto* rightOut = out[0];

    for (size_t i = 0; i < size; ++i)
    {
        auto const sample = oscillator();
        leftOut[i]        = sample;
        rightOut[i]       = sample;
    }
}

int main(void)
{
    oscillator.setShape(mc::OscillatorShape::Sine);
    oscillator.setFrequency(440.0F);
    oscillator.setSampleRate(patch.AudioSampleRate());

    patch.Init();
    patch.StartAudio(process);
    while (true) {}
}
