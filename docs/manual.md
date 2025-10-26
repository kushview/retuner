# reTuner User Manual

## Overview

**reTuner** is a precision audio pitch shifting plugin designed to convert music between different tuning standards. Whether you're working with audio tuned to standard A440 Hz or any other reference frequency, reTuner allows you to seamlessly shift the pitch to alternative tuning standards like A432 Hz, which is popular in meditation music, wellness audio, and alternative music circles.

reTuner uses high-quality pitch shifting algorithms to maintain audio quality while preserving the character and formants of the original recording. The plugin is available in multiple formats (VST3, LV2, CLAP) and as a standalone application with additional audio file playback and export capabilities.

---

## Plugin Parameters

### SOURCE A4
**Range:** 380.0 Hz - 460.0 Hz  
**Default:** 440.0 Hz

Sets the reference frequency (A4) of the source audio. This is the tuning standard your input audio is currently using. Common values include:
- **440 Hz** - Modern standard pitch (ISO 16)
- **441 Hz** - Common in some European contexts
- **432 Hz** - Alternative tuning standard
- **444 Hz** - Higher pitch standard

Adjust this parameter to match the tuning of your source material. The plugin will calculate the required pitch shift based on the difference between this value and the Target A4 frequency.

### TARGET A4
**Range:** 380.0 Hz - 460.0 Hz  
**Default:** 432.0 Hz

Sets the desired reference frequency (A4) for the output audio. This is the tuning standard you want to convert your audio to. The plugin will pitch-shift the entire audio spectrum proportionally to achieve this target tuning.

Common target values:
- **432 Hz** - Popular in meditation, healing, and new age music
- **440 Hz** - Standard if converting from alternative tunings back to modern standard
- **415 Hz** - Baroque period tuning
- **435 Hz** - French standard (pre-1939)

### VOLUME
**Range:** -60.0 dB - +12.0 dB  
**Default:** 0.0 dB

Controls the output level of the processed audio. Use this to compensate for any perceived volume changes after pitch shifting, or to adjust the overall output level to match your mix.

- **0 dB** - Unity gain (no volume change)
- **Negative values** - Reduce output volume
- **Positive values** - Boost output volume (use carefully to avoid clipping)

---

## Standalone Application

The standalone version of reTuner includes all plugin functionality plus additional features for working with audio files directly.

### Media Player Controls

#### File Loading
Use the **LOAD** button or **File → Open...** menu to load an audio file. Supported formats include WAV, AIFF, MP3, FLAC, and other common audio formats supported by your system.

Once loaded, the filename appears in the file display area above the transport controls.

#### Transport Controls

- **PLAY** - Start playback from the current position
- **PAUSE** - Pause playback (press PLAY to resume)
- **STOP** - Stop playback and return to the beginning

#### Position Control

The position slider shows playback progress through the file. You can click or drag the slider to jump to any position in the audio file. The time display shows current position and total duration in MM:SS format.

### Menu Bar

#### File Menu

- **Open...** - Load an audio file for playback and processing
- **Reset Defaults** - Reset all plugin parameters to their default values (440 Hz → 432 Hz at 0 dB)
- **Export...** - Export the currently loaded audio file with the current pitch shift settings applied (available only when a file is loaded)
- **Preferences...** - Access application preferences (audio device settings, etc.)
- **Quit** - Exit the application

#### Help Menu

- **User Manual** - Open this user manual
- **About ReTuner** - Display version and copyright information

### Export Functionality

When an audio file is loaded, you can export a processed version with your current pitch shift settings:

1. Load an audio file using **File → Open...**
2. Adjust the SOURCE A4 and TARGET A4 parameters to configure your desired pitch shift
3. Optionally adjust the VOLUME parameter
4. Select **File → Export...**
5. Choose a destination filename and format
6. The exported file will contain your audio with the pitch shift permanently applied

This is useful for batch processing, creating alternate versions of tracks, or preparing files for distribution in alternative tuning standards.

---

## Workflow Examples

### Converting Standard 440 Hz Music to 432 Hz

1. Set **SOURCE A4** to 440.0 Hz
2. Set **TARGET A4** to 432.0 Hz
3. Set **VOLUME** to 0.0 dB (or adjust as needed)
4. In the standalone app: Load your file, preview the result, and export if satisfied
5. In the plugin: Process audio in real-time through your DAW

The plugin will shift the pitch down by approximately -32 cents (about 1.8% frequency reduction).

### Converting European 441 Hz to 432 Hz

1. Set **SOURCE A4** to 441.0 Hz
2. Set **TARGET A4** to 432.0 Hz
3. Adjust **VOLUME** as needed
4. Process or export your audio

### Converting from 432 Hz Back to Standard 440 Hz

1. Set **SOURCE A4** to 432.0 Hz
2. Set **TARGET A4** to 440.0 Hz
3. Adjust **VOLUME** as needed
4. Process or export your audio

---

## Tips & Best Practices

### Audio Quality
- reTuner uses high-quality pitch shifting algorithms designed to preserve audio quality
- Small pitch shifts (under 50 cents) generally produce transparent results
- Larger shifts may introduce subtle artifacts depending on source material

### Monitoring Levels
- Watch your output levels after pitch shifting
- Use the VOLUME parameter to compensate for any perceived loudness changes
- Avoid clipping by keeping peaks below 0 dB

### Real-time vs. Offline Processing
- In DAWs: reTuner processes audio in real-time with low latency
- In standalone: Export to disk for offline processing and permanent file conversion

### Source Material
- Works best with full mixes and complete recordings
- Can be used on individual tracks, but consider processing the entire mix for consistency
- Ensure you know the original tuning standard of your source material for accurate conversion

---

## Technical Specifications

- **Pitch Shift Range:** ±3 semitones (via frequency range 380-460 Hz)
- **Volume Range:** -60 dB to +12 dB
- **Processing:** High-quality pitch shifting with formant preservation
- **Latency:** Low latency suitable for real-time monitoring
- **Precision:** 0.1 Hz frequency adjustment resolution

---

## Support & Information

For more information, updates, and support, visit: [Kushview](https://kushview.net)

---

*Copyright (c) 2025 Kushview. All rights reserved.*
