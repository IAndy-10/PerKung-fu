# PerKung-fu

An experimental percussion synthesizer that combines physical modelling with computer vision preset control.
Formats: **VST3 · AU · Standalone** — built with JUCE 8 and Svelte.

## Download

**Latest Release:** [PerKung-fu v0.1.0](https://github.com/IAndy-10/PerKung-fu/releases/latest)

### Quick Install (macOS)

1. **Download** the `.zip` from [Releases](https://github.com/IAndy-10/PerKung-fu/releases)
2. **Unzip** and copy the plugins:
   - `PerKung-fu.vst3` → `~/Library/Audio/Plug-Ins/VST3/`
   - `PerKung-fu.component` → `~/Library/Audio/Plug-Ins/Components/`
   - `PerKung-fu.app` → `/Applications/` (optional, for standalone)
3. **Rescan** plugins in your DAW (restart DAW if needed)

### Supported Formats
- **VST3** (macOS 11+)
- **AU** (macOS 10.13+)
- **Standalone** (macOS 11+)

---

## Getting Started

### First Use (DAW)

1. Load **PerKung-fu** as an insert on any track, or open the Standalone app.
2. Open the plugin window — the embedded UI loads automatically.
3. Connect a contact mic or use any microphone input through your audio interface.

### Setting Up the Contact Mic

1. Plug your contact mic into your audio interface.
2. Route that input channel into the plugin (in DAW: set the track input accordingly; in Standalone: select your interface as the input device).
3. Adjust **Mic Gain** and **Threshold** so that hits trigger the synthesis without false triggers from ambient noise.
   - Start with **Threshold** around −30 dB and raise it until room noise no longer triggers the synth.
   - Tap the surface and confirm the FFT meter responds.

### Enabling Object Detection (Camera Presets)

1. Click the **Camera** button in the UI.
2. macOS will prompt for **camera permission** — click Allow. (This happens only once per app.)
3. The "Detected Objects" panel shows what the camera sees and the confidence score.
4. Hold a supported object in frame for ~0.6 s — the preset loads automatically and all parameters update.

### Supported objects

| Object | Character |
|--------|-----------|
| Banana | Long warm pluck — slow decay, low damping, +10 st tuning |
| Apple  | Short hard knock — fast decay, bright, percussive, −15 st tuning |

The plugin works without a camera — presets just won't switch automatically.

### Recommended starting point

- **Threshold**: −30 to −20 dB (adjust to your mic sensitivity)
- **Decay**: 0.5–1.5 s for punchy hits
- **Damp**: 40–60% for a balanced tone
- **Strike**: 60–80% for a natural percussive attack

---

## Project goal

PerKung-fu is a playable, immersive plugin for producers and sound designers. It blends punchy rhythm textures, modular performance controls, and a distinctive visual presentation — bridging analog-style percussion design with a digital performance interface.

The plugin stands out by mixing physical model synthesis with real-time object recognition as an interaction layer, exploring new ways to control a drum instrument beyond knobs and pads.

---

## Inspirations

The UI aesthetic is directly inspired by *Neon Genesis Evangelion* — the system terminals and interfaces. Visual references were gathered from Pinterest and iterated through Claude's artifact canvas. The CRT bloom, scanlines, and glow were explicit design decisions to land on a sci-fi old-school terminal feel.

---

## What it does

PerKung-fu listens to a microphone, detects percussive hits (onset detection), and synthesizes each hit using a **Karplus-Strong string model**. The sound is shaped by a set of parameters — tuning, decay, damping, strike hardness, and more.

What makes it unusual: a **camera feed runs MediaPipe object detection in real time**. When a recognized object (like a banana or an apple) stays in frame for ~0.6 seconds, the plugin automatically loads a matching synthesis preset and updates all parameters.

---

## Signal flow

```
Contact mic → Onset detection → Karplus-Strong synthesis → Output
                                        ^
                    Camera → MediaPipe → Preset (tuning, decay, damp…)
```

---

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| Tuning | -24 .. +24 st | Pitch of the synthesized string |
| Decay | 0.05 .. 4.0 s | How long the string rings |
| Damp | 0 .. 100 % | High-frequency damping (brightness) |
| Strike | 0 .. 100 % | Exciter hardness at the moment of impact |
| Attenuation | 0 .. 100 % | Body resonance attenuation |
| L/Cut | 20 .. 20k Hz | Low-cut filter on the exciter |
| Mic Gain | 0 .. 2× | Input gain before onset detection |
| Out Gain | -36 .. 0 dB | Output level |
| Threshold | -60 .. 0 dB | Onset detection gate |

---

## Camera presets

When the camera detects a supported object and it remains stable for 10 consecutive frames (~0.66 s), the corresponding preset is loaded automatically. The "Detected Objects" panel shows the active target and confidence score.

| Object | Tuning | Character |
|--------|--------|-----------|
| Banana | +10 st | Long warm pluck — slow decay, low damping |
| Apple  | -15 st | Short hard knock — fast decay, bright, percussive |

To add a new preset, edit `WebUI/src/presets.ts` and add the object label to the `ALLOWED` set in `WebUI/src/components/ObjectDetector.svelte`.

---

## Architecture

```
Source/                  C++ / JUCE
  PluginProcessor.cpp    DSP: onset detection, Karplus-Strong, FFT spectrum
  PluginEditor.cpp       WebView host, 30 Hz timer, JS↔C++ bridge
  WebviewBridge.h        Intercepts juce:// URLs, calls APVTS
  DSP/DrumVoice.h        Karplus-Strong voice

WebUI/src/               TypeScript / Svelte
  App.svelte             Main UI, stabilization logic, preset trigger
  bridge/bridge.ts       JS→C++ parameter queue (one message at a time)
  state/store.ts         Normalized 0–1 parameter stores
  presets.ts             Preset definitions
  components/
    ObjectDetector.svelte  MediaPipe inference loop (15 fps)
    Camera.svelte          getUserMedia stream
    ParamSlider.svelte     Individual parameter control
```

### JS ↔ C++ communication

- **C++ → JS**: `evaluateJavascript("window.setParameterValue(id, value)")` at 30 Hz
- **JS → C++**: `window.location.href = "juce://setparameter?name=id&value=v"` intercepted by `WebViewBridge::pageAboutToLoad()`. Messages are queued and sent one at a time (16 ms apart) to avoid WKWebView dropping concurrent navigation requests.

---

## Building

### Requirements

- macOS (Apple Silicon or Intel)
- Xcode Command Line Tools
- CMake 3.22+
- Node.js + npm

### Steps

```bash
# 1. Install WebUI dependencies (first time only)
cd WebUI && npm install && cd ..

# 2. Configure
mkdir build && cd build
cmake ..

# 3. Build
make -j$(sysctl -n hw.logicalcpu)
```

`npm run build` is run automatically by CMake whenever a WebUI source file changes — no manual step needed.

The plugin is copied to `~/Library/Audio/Plug-Ins/` after each successful build (`COPY_PLUGIN_AFTER_BUILD TRUE`).

---

## LLM use

Claude Sonnet 4.6 (Claude.ai and Claude Code) was used throughout development to speed up the process and to adapt patterns from other functional projects into this codebase. LLM-assisted work includes: C++ DSP architecture and JUCE integration, the JS↔C++ bridge design, CMake configuration, and debugging the WebView parameter pipeline.

The UI was designed separately. Visual references were gathered from Pinterest and iterated through Claude's artifact canvas until the aesthetics felt right. The CRT bloom and scanline effects were an explicit design decision and the blur/glow layers were set since the beggining.


