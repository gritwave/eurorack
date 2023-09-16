# gritwave - eurorack

[![Package](https://github.com/gritwave/eurorack/actions/workflows/package.yml/badge.svg)](https://github.com/gritwave/eurorack/actions/workflows/package.yml)
[![codecov](https://codecov.io/gh/gritwave/eurorack/graph/badge.svg?token=7zVMQmr3Rb)](https://codecov.io/gh/gritwave/eurorack)

## Projects

|         Name         |         Description         |                                  Hardware                                  |
| :------------------: | :-------------------------: | :------------------------------------------------------------------------: |
| [Astra](./src/astra) | Multi-Effect Mono to Stereo | [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) |
| [Hades](./src/hades) | Multi-Mono Noise/Distortion | [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) |
|  [Kyma](./src/kyma)  |    Wavetable Oscillator     | [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) |

## Development

### Create Release

```sh
git commit -m "Releasing v0.1.0"
git tag v0.1.0
git push --tags
```

### Resources

- [Pink Noise](https://www.firstpr.com.au/dsp/pink-noise)
- [cytomic.com: DynamicSoothing](https://cytomic.com/files/dsp/DynamicSmoothing.pdf)
- [Mutable Instruments' Eurorack Modules: github.com/pichenettes/eurorack](https://github.com/pichenettes/eurorack)
