#include "daisy_patch_sm.h"

daisy::patch_sm::DaisyPatchSM patch;

void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }
}

int main(void)
{
    patch.Init();
    patch.StartAudio(AudioCallback);
    while (true) {}
}
