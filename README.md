# Eurorack

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](./LICENSE.txt)
[![Package](https://github.com/gritwave/eurorack/actions/workflows/package.yml/badge.svg)](https://github.com/gritwave/eurorack/actions/workflows/package.yml)
[![codecov](https://codecov.io/gh/gritwave/eurorack/graph/badge.svg?token=7zVMQmr3Rb)](https://codecov.io/gh/gritwave/eurorack)
[![GitHub Release](https://img.shields.io/github/release/gritwave/eurorack.svg?style=flat)](https://github.com/gritwave/eurorack/releases/latest)
[![Github All Releases](https://img.shields.io/github/downloads/gritwave/eurorack/total.svg)](https://github.com/gritwave/eurorack/releases/latest)

This repository contains both the library and firmware code for all gritwave eurorack modules. Based on the [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) hardware.

## Firmware

Latest versions can be downloaded [here](https://github.com/gritwave/eurorack/releases/latest)

|            Name            |         Description         |                                  Hardware                                  |
| :------------------------: | :-------------------------: | :------------------------------------------------------------------------: |
|     [Ares](./src/ares)     |  Multi-Mono Amp Simulation  | [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) |
|    [Astra](./src/astra)    | Multi-Effect Mono to Stereo | [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) |
|   [Hermas](./src/hermas)   |     Multi-Mono Dynamics     | [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) |
|     [Kyma](./src/kyma)     |    Wavetable Oscillator     | [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) |
| [Poseidon](./src/poseidon) | Multi-Mono Noise/Distortion | [Electrosmith Patch.init()](https://www.electro-smith.com/daisy/patchinit) |

## Library

```cmake
# C++20 minimum
add_subdirectory(path/to/eurorack/3rd_party/tetl/include)
add_subdirectory(path/to/eurorack/lib)

# Header only dsp library
target_link_libraries(YourTarget PRIVATE gritwave::grit)

# Static library containing firmware dsp modules (includes gritwave::grit)
target_link_libraries(YourTarget PRIVATE gritwave::eurorack)
```

### Documentation

```sh
doxygen Doxyfile
```

## Development

### Create Release

```sh
# Bump VERSION & Doxygen PROJECT_NUMBER
git commit -am "Release v0.1.0"
git tag v0.1.0
git push
git push --tags
```

### Compiler Explorer

```sh
g++ -O3 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -ffast-math
clang++ -O3 --target=arm-none-eabi -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -ffast-math
```

### Resources

- [Pink Noise](https://www.firstpr.com.au/dsp/pink-noise)
- [cytomic.com: DynamicSoothing](https://cytomic.com/files/dsp/DynamicSmoothing.pdf)
- [Mutable Instruments' Eurorack Modules: github.com/pichenettes/eurorack](https://github.com/pichenettes/eurorack)
