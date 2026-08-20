# Release Notes

## v0.1.0 (Initial Release)

**Date:** August 20, 2026

### Features
- Physical modelling percussion synthesizer with 8 drum voices
- Computer vision preset control via webcam and MediaPipe object detection
- Real-time object detection maps detected objects to synthesis presets
- Contact mic input support for acoustic triggering
- FFT spectrum analyzer and real-time level meters
- Svelte WebUI embedded via JUCE WebBrowserComponent
- VST3, AU, and Standalone formats

### Technical
- Built with JUCE 8.0.4 + C++17 + CMake 3.22+
- Svelte 5 + TypeScript + Tailwind CSS frontend
- Self-contained binary — no external web server or internet required at runtime
- Camera and microphone permissions included in plugin bundle

### System Requirements
- macOS 11.0 or later
- Intel or Apple Silicon
- Built-in or external webcam (required for object detection preset control)
- Microphone or audio interface (required for contact mic triggering)

### Known Limitations
- macOS only (Windows planned)
- Webcam required for object detection features; plugin works without it but preset mapping will be inactive

### Download
[Download PerKung-fu v0.1.0](https://github.com/IAndy-10/PerKung-fu/releases/tag/v0.1.0)
