# reTuner

reTuner is a precision audio pitch shifting plugin designed to convert music between different tuning standards. It allows you to seamlessly shift the pitch of audio from one reference frequency (e.g., A440) to another (e.g., A432), preserving the character and formants of the original recording.

![Screenshot](docs/screenshot.png)

Built with [JUCE](https://juce.com/) and [Rubber Band Library](https://breakfastquay.com/rubberband/).

## Features

- **High-Quality Pitch Shifting**: Uses the Rubber Band Library for artifact-free pitch shifting.
- **Precise Tuning Control**:
  - **Source A4**: Set the reference frequency of your input audio (380Hz - 460Hz).
  - **Target A4**: Set the desired output reference frequency.
- **Multi-Format**: Available as VST3, LV2, CLAP, AU and Standalone application.
- **Cross-Platform**: Compatible with Windows, macOS, and Linux.

## Building

reTuner uses CMake for its build system.

### Prerequisites

- CMake 3.21 or higher
- C++20 compatible compiler
- Ninja (optional, but recommended for faster builds)

### Build Instructions

1.  **Configure the project:**

    ```bash
    cmake -B build -G Ninja
    ```
    *(Omit `-G Ninja` if you prefer to use your default generator, e.g., Makefiles or Visual Studio)*

2.  **Build the plugin:**

    ```bash
    cmake --build build
    ```

    This will compile the VST3, LV2, CLAP plugins and the Standalone application.

3.  **Run Tests:**

    ```bash
    ctest --test-dir build
    ```

## Installation

After building, the artifacts can be found in the `reTuner_artefacts` directory for plugins or `build/reTunerApp_artefacts` for the standalone application.

## License

Copyright (c) 2025 Kushview, LLC.
Licensed under the GPL-3.0-or-later license. See `LICENSE.txt` for details.
