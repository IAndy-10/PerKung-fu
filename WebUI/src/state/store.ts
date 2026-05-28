import { writable } from 'svelte/store';
import type { ParameterId } from '../types/parameters';

// Normalized 0-1 parameter stores. Defaults match JUCE NormalisableRange defaults.
// Skewed params: norm = ((val - min) / (max - min)) ^ (1/skew)
export const params = {
    tuning:     writable(0.5),   // -24..24 linear, default 0 → 0.5
    decay:      writable(0.013), // 0.05..4.0 skew=0.5, default 0.5s → 0.013
    damp:       writable(0.3),   // 0..1 linear, default 0.3
    strike:     writable(0.5),   // 0..1 linear, default 0.5
    atten:      writable(0.0),   // 0..1 linear, default 0.0
    lcut:       writable(1.0),   // 0..1 linear → 20..20kHz, default 1.0
    mic_gain:   writable(0.5),   // 0..2 linear, default 1.0 → 0.5
    out_gain:   writable(0.983), // -36..0 skew=2.5, default -1.5 dB → 0.983
    threshold:  writable(0.644), // -60..0 skew=2.5, default -40 dB → 0.644
};

// Real-time audio level stores — updated by C++ at 30 Hz
export const inputLevel  = writable(0.0);
export const outputLevel = writable(0.0);

export function setParameterValue(id: ParameterId, value: number) {
    (params as Record<string, { set: (v: number) => void }>)[id]?.set(value);
}
