# PerKung-fu — Architecture Diagram Data

**Date:** 2026-09-29
**Version:** 0.1.0

## Layers

### UI Layer
- Svelte 5 + TypeScript, Tailwind CSS, Vite 7.3.1
- WKWebView (macOS), single-file HTML embedded via `juce_add_binary_data`
- MediaPipe object detection (camera-based preset switching), FFT spectrum visualizer, input/output meters
- 34 Svelte components (ParamSlider, Camera, ObjectDetector, GestureMap, XYPad, etc.)

### Bridge
- **JS to C++:** Legacy URL intercept (`juce://setparameter?name=<id>&value=<norm>`) via `WebViewBridge::pageAboutToLoad()`. Queued at 16 ms intervals to avoid WKWebView throttling.
- **C++ to JS:** `evaluateJavascript()` called from a 30 Hz `timerCallback()`. Pushes parameter values, input/output peaks, and 40-bin FFT spectrum data.
- **Threading:** JS messages arrive on message thread (safe for `setValueNotifyingHost`). Audio thread reads only via `getRawParameterValue()` atomic pointers. C++ to JS dispatched from message thread timer.

### DSP Layer

```
Contact Mic Input (stereo)
  |
  +-- [Mic Gain]         Linear multiplier (0-2x) applied to input
  |
  +-- [OnsetDetector]    High-pass filter (1st-order difference)
  |                      3 ms peak-hold window
  |                      Threshold comparison (dB-scaled)
  |                      50 ms refractory cooldown
  |                      Returns: velocity [0,1] or -1 (no onset)
  |
  +-- [DrumVoice]        Modal synthesis (8 circular membrane modes)
  |     |                Bessel zero ratios: 1.000, 1.593, 2.135, 2.295,
  |     |                                   2.653, 2.917, 3.156, 3.500
  |     |                Per-mode frequency-dependent decay
  |     |                Exciter falloff controlled by Strike parameter
  |     |                1-pole low-pass filter (L/Cut: 20 Hz - 20 kHz)
  |     |                Attenuation: 0-95% amplitude reduction
  |     |                Monophonic with 1 ms crossfade on retrigger
  |
  +-- [Output Gain]      dB-scaled master output (-36 to 0 dB)
  |
  +-- [FFT Accumulation] 1024-point Hann-windowed FFT
  |                      40 log-spaced bins (20 Hz - 20 kHz)
  |                      ~43 Hz update rate, stored in atomic array
  |
  Output Buffer (mono synth copied to all stereo channels)
```

No dry/wet mixing — synthesis fully replaces input signal.

### I/O
- **Audio:** Stereo in (contact mic) -> Stereo out (synthesized output)
- **MIDI:** Accepts MIDI input (currently unused; onset detection triggers synthesis). No MIDI output.
- **Tail length:** 4.0 seconds
- **Permissions:** Microphone (contact mic input), Camera (object detection for presets)

## Diagram

```
    +------------------------------+
    |          UI Layer            |
    |  Svelte 5 + TypeScript      |
    |  Camera / Object Detection  |
    |  Spectrum / Meters          |
    +-------------+----------------+
                  | juce:// URL intercept (JS->C++)
                  | evaluateJavascript (C++->JS, 30 Hz)
                  v
    +---------------------------------------------+
    |              JUCE DSP (C++17)                |
    |                                              |
    |  . Mic Gain (input scaling)                  |
    |  . OnsetDetector (transient detection)       |
    |  . DrumVoice (8-mode modal synthesis)        |
    |  . Output Gain (master level)                |
    |  . FFT Spectrum (1024-pt, 40 bins)           |
    |                                              |
    |  Stereo In ----->          -----> Stereo Out  |
    +---------------------------------------------+
```

## Parameters

| ID | Name | Range | Default | Unit | Skew |
|---|---|---|---|---|---|
| tuning | Tuning | -24 to +24 | 0.0 | semitones | 1.0 |
| decay | Decay | 0.05 to 4.0 | 0.5 | seconds | 0.5 |
| damp | Damp | 0.0 to 1.0 | 0.3 | % | 1.0 |
| strike | Strike | 0.0 to 1.0 | 0.5 | % | 1.0 |
| atten | Attenuation | 0.0 to 1.0 | 0.0 | % | 1.0 |
| lcut | L/Cut | 0.0 to 1.0 | 1.0 | % (log Hz) | 1.0 |
| mic_gain | Mic Gain | 0.0 to 2.0 | 1.0 | x | 1.0 |
| out_gain | Out Gain | -36 to 0 | -1.5 | dB | 2.5 |
| threshold | Threshold | -60 to 0 | -40 | dB | 2.5 |

## Key Files

| Path | Purpose |
|---|---|
| Source/PluginProcessor.cpp | processBlock: onset detection -> synthesis -> FFT |
| Source/PluginProcessor.h | Processor class, OnsetDetector |
| Source/PluginEditor.cpp | WebView host, 30 Hz parameter push timer, camera permissions |
| Source/WebviewBridge.h | juce:// URL intercept handler |
| Source/ParameterIDs.h | APVTS parameter ID constants |
| Source/DSP/DrumVoice.h | Modal synthesis engine (8 Bessel modes) |
| WebUI/src/App.svelte | Main UI: params, camera, spectrum, meters |
| WebUI/src/bridge/bridge.ts | JS->C++ parameter change queue |
| WebUI/src/state/store.ts | Svelte writable stores (normalized 0-1) |
| WebUI/src/presets.ts | Object-mapped presets (banana, apple) |
