# GitHub Copilot Instructions for Audio Plugin JUCE Project

## Project Overview
ReTuner is a cross-platform audio plugin built with JUCE that generates test tones for audio engineers and producers. The project builds VST3, LV2, CLAP, and standalone formats across multiple platforms.

## Code Style Guidelines
- Use modern C++ (C++20) patterns and idioms
- Follow JUCE's naming conventions (`camelCase` for variables/functions, `PascalCase` for classes)
- Don't use `<JuceHeader.n>`, it isn't ever needed in this project.
- Maintain a clean separation between DSP code and UI code
- Optimize for real-time audio performance - no blocking operations in the audio thread
- Document all public methods with clear, concise comments
- Private member variables should start with underscore. e.g. `_varName`
- Getter functions should avoid the word "get". e.g. do `obj.something()` insteadof `obj.getSomething()`
- File names sould be `alllowercase` no spaces, no dashes, no underscores.
- Use the file extension `.hpp` for c++ headers.
- All code should live in the `src` directory or a subdirectory of `src`

## JUCE Look and Feel
When writing look and feel code for juce, make sure to research `LookAndFeelMethods` classes for particular methods to override.

## Avoid JUCE Deprecated Code
- Utilize juce::FontOptions when manually dealing with Fonts
- Utilize `juce::TextLayout` or `juce::GlyphArrangment` static string methods to calculate sizes needed in text rendering.

## Audio Domain Knowledge
When working with this codebase, be aware of these critical audio development concepts:
- Real-time audio constraints (no memory allocation, no locks, no system calls in the audio thread)
- Audio buffer processing patterns for zero latency and minimum CPU usage
- Understanding of sample rates, buffer sizes, and their impact on performance
- Thread safety considerations between the audio thread and the message thread
- Plugin format requirements (VST3, LV2, CLAP) and their unique constraints

## Build System
- CMake-based build that generates platform-specific project files
- Cross-platform considerations for Windows, macOS, and Linux
- Bundle packaging via Python scripts (`utils/artifacts.py`)
- Make sure to `cd` in to the build directory when using the cmake option `--build .`

## Key Components
- **processor.cpp/.hpp**: Audio DSP code and parameter handling
- **editor.cpp/.hpp**: GUI implementation
- **src/app/*.***: Standalon application code
- **CMakeLists.txt**: Build configuration

## Performance Considerations
- Audio processing must be efficient and avoid allocations
- GUI should not block the audio thread
- Plugin should handle sample rate and buffer size changes gracefully
- All operations in the audio thread must be real-time safe

## Common Tasks
- Adding new parameters to the processor
- Updating the UI to reflect new parameters
- Optimizing DSP algorithms
- Fixing platform-specific issues
- Packaging builds for distribution

## Restrictions
- Do not ever modify dependencies under the `deps/**` directory.

When suggesting code changes, always consider real-time audio constraints, the KISS principle, and cross-platform compatibility. 
