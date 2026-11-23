# PerKung-fu — Performance Evaluation

**Date:** 2026-09-29
**Version:** 0.1.0

## Plugin Overview

| Field | Value |
|---|---|
| Plugin name | PerKung-fu |
| CMake target | PerKungFu |
| Company | YoTerra |
| Plugin code | Pkfu |
| Manufacturer code | Smps |
| Parameters | 9 (all Float) |
| Audio I/O | Stereo in / Stereo out |
| MIDI | Input accepted (unused), no output |
| DSP algorithm | Modal synthesis (8 Bessel modes) triggered by contact mic onset detection |
| UI stack | Svelte 5 + TypeScript + Tailwind CSS, embedded via WKWebView |
| Tail length | 4.0 s |

## Test Machine

| Field | Value |
|---|---|
| Model | MacBook Air |
| Chip | Apple M1 |
| Memory | 8 GB |
| OS | macOS 26.5 (Build 25F71) |

## Build Artefacts

| Format | Path | Built |
|---|---|---|
| VST3 | `build/PerKungFu_artefacts/Release/VST3/PerKung-fu.vst3` | Yes |
| AU | `build/PerKungFu_artefacts/Release/AU/PerKung-fu.component` | Yes |
| Standalone | `build/PerKungFu_artefacts/Release/Standalone/PerKung-fu.app` | Yes |

## pluginval Results

| Format | Strictness | Block Sizes | Sample Rate | Result |
|---|---|---|---|---|
| VST3 | 5 | 64, 128, 256, 512 | 48 kHz | PASS |
| AU | 5 | 64, 128, 256, 512 | 48 kHz | PASS |

**AU notes:** PerKung-fu registers as a music effect (`aumf`), not `aufx`. The earlier failure came from validating with the wrong type code. `auval -v aumf Pkfu Smps` reports AU VALIDATION SUCCEEDED and pluginval strictness 5 passes on the installed component (re-checked 2026-11-22).

## CPU Usage

| Buffer Size | Sample Rate | CPU % | Method |
|---|---|---|---|
| 512 | 48 kHz | TBD | DAW meter |
| 256 | 48 kHz | TBD | DAW meter |
| 128 | 48 kHz | TBD | DAW meter |
| 64 | 48 kHz | TBD | DAW meter |

**Estimated budget:** <1% at 512/48 kHz on M1 (onset detector + 8-mode modal synth + 1024-pt FFT). Actual DAW measurements require manual testing with audio input.

## DAW Compatibility

| DAW | Format | Loads | Audio | Automation | Preset Save/Restore |
|---|---|---|---|---|---|
| Logic Pro | AU | TBD | TBD | TBD | TBD |
| Reaper | VST3 | TBD | TBD | TBD | TBD |

## Real-Time Safety (processBlock)

| Check | Status |
|---|---|
| ScopedNoDenormals | Yes |
| No heap allocations | Yes |
| No std::string | Yes |
| No file I/O or logging | Yes |
| Parameters via atomic pointers | Yes |
| Buffers pre-allocated in prepareToPlay | Yes |
| Denormal flushing in DSP | Yes (amplitude < 1e-7f returns 0) |
| SmoothedValue on parameters | No (potential clicks on fast automation) |

## Known Limitations

1. **Monophonic** — single voice with 1 ms crossfade on retrigger
2. **MIDI input accepted but unused** — onset detection is the sole trigger mechanism
3. **Object detection limited** — only 2 presets (banana, apple) mapped
4. **No preset save/load UI** — presets are code-defined only
