# Poseidon

## Features

- Dual Mono I/O
- Random Variations per Channel
- **Texture**
  - White Noise
  - Pink Noise
  - Brown Noise
  - Samples?
- **Amp**
  - Convolution
  - Distortion
  - Saturation
  - Tanh
  - Sin
  - AirWindows
  - digitaldreams (Shape)
- **Compressor**
  - Attack, Release, Mix
- Attack/Release CV
  - Compressor
  - Noise Env Follower Fast
- Sidechain CV
  - Compressor Intensity
- Morph CV
  - Texture Morph

## Mapping

|   Panel Label   |         Function         |                      Description                      |
| :-------------: | :----------------------: | :---------------------------------------------------: |
|     IN Left     |       Channel 1 In       |                                                       |
|    IN Right     |       Channel 2 In       |                                                       |
|    OUT Left     |      Channel 1 Out       |                                                       |
|    OUT Right    |      Channel 2 Out       |                                                       |
| gate_out_1 (B5) |      Gate XOR Ouput      |                                                       |
| gate_out_2 (B6) | Gate XOR Output Inverted |                                                       |
|   button (B7)   |       Next Preset        |      Switches depending on switch (B8) position       |
|   switch (B8)   |      Engine Select       |                    Texture or Amp                     |
| gate_in_1 (B10) |       Gate Input A       |                                                       |
| gate_in_2 (B9)  |       Gate Input B       |                                                       |
|   knob (CV_1)   |      Texture Amount      |                                                       |
|   knob (CV_2)   |      Texture Morph       |                                                       |
|   knob (CV_3)   |        Amp Drive         |                                                       |
|   knob (CV_4)   |   Compressor Intensity   |                  Threshold and Ratio                  |
|      CV_5       |      Texture Morph       |                                                       |
|      CV_6       |  Compressor Side-Chain   |                                                       |
|      CV_7       |          Attack          | Texture Envelope-Follower & Compressor Attack/Release |
|      CV_8       |         Release          | Texture Envelope-Follower & Compressor Attack/Release |
|    CV_OUT_1     | Envelope-Follower Output |                                                       |
|    CV_OUT_2     |                          |                                                       |
