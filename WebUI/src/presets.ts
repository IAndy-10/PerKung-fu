import type { ParameterId } from './types/parameters';

export type Preset = Record<ParameterId, number>;

// Normalized (0..1) values — match JUCE NormalisableRange definitions in PluginProcessor.cpp.
// Decay skew=0.5:  val = 0.05 + 3.95 * sqrt(norm)
// OutGain skew=2.5: val = -36 + 36 * norm^2.5
// Threshold skew=2.5: val = -60 + 60 * norm^2.5

export const PRESETS: Record<string, Preset> = {
    // Long warm pluck — sustained, muted, low-cut kept open
    banana: {
        tuning:    0.708,  // +10 semitones
        decay:     0.5,   // ~0.98 s
        damp:      0.5,    // 50 %
        strike:    0.5,    // 50 %
        atten:     0.1,    // 10 %
        lcut:      0.3,    // ~159 Hz
        mic_gain:  0.5,    // 1.0×
        out_gain:  0.983,  // ~-1.5 dB
        threshold: 0.644,  // ~-40 dB
    },

    // Short hard knock — bright, percussive, tight
    apple: {
        tuning:    0.188,  // -15 semitones
        decay:     0.55,  // ~0.98 s
        damp:      0.15,   // 15 %
        strike:    0.8,    // 80 %
        atten:     0.3,    // 30 %
        lcut:      0.5,    // ~632 Hz
        mic_gain:  0.5,    // 1.0×
        out_gain:  0.983,  // ~-1.5 dB
        threshold: 0.644,  // ~-40 dB
    },
};
