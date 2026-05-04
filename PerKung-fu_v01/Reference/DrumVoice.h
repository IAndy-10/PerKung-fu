#pragma once
// Modal synthesis DSP engine — direct port of the Rust dsp.rs.
//
// Circular membrane modes via Bessel function zero frequency ratios.
// Source: "Physical Modeling of Percussion Instruments" — Gärder, Chalmers 2005
//
// Decay (eq.3.19): λ = (1/1000)^(1 / (Fs · T60))
// Freq-dependent damp: mode_decay = base_decay ^ (1 + damp · ratio)

#include <array>
#include <algorithm>
#include <cmath>

static constexpr int   NUM_MODES = 8;
static constexpr float MODE_RATIOS[NUM_MODES]     = { 1.000f, 1.593f, 2.135f, 2.295f,
                                                       2.653f, 2.917f, 3.156f, 3.500f };
static constexpr float BASE_AMPLITUDES[NUM_MODES] = { 1.00f,  0.75f,  0.55f,  0.45f,
                                                       0.35f,  0.28f,  0.22f,  0.18f };
static constexpr float TWO_PI = 6.28318530718f;

struct Mode
{
    float phase     = 0.0f;
    float amplitude = 0.0f;
    float phaseInc  = 0.0f;
    float decay     = 0.999f;

    void setup (float freq, float amp, float decayVal, float sampleRate)
    {
        phase     = 0.0f;
        amplitude = amp;
        phaseInc  = TWO_PI * freq / sampleRate;
        decay     = decayVal;
    }

    float nextSample()
    {
        if (std::abs (amplitude) < 1e-7f) return 0.0f;
        float s = amplitude * std::sin (phase);
        phase += phaseInc;
        if (phase >= TWO_PI) phase -= TWO_PI;
        amplitude *= decay;
        return s;
    }
};

class DrumVoice
{
public:
    DrumVoice() = default;

    void setSampleRate (float sr) { sampleRate = sr; }

    void trigger (int note, float velocity, float tuning,
                  float decay, float damp, float strike)
    {
        // If voice is active, schedule a fast fade-out (1 ms) before retriggering.
        bool wasActive = false;
        for (auto& m : modes)
            if (std::abs (m.amplitude) > 1e-5f) { wasActive = true; break; }

        if (wasActive)
        {
            int fadeSamples = std::max (static_cast<int> (sampleRate * 0.001f), 1);
            float fadeDecay = std::pow (1e-4f, 1.0f / (float) fadeSamples);
            for (auto& m : modes)
                m.decay = fadeDecay;
            fadeCountdown   = fadeSamples;
            pendingNote     = note;
            pendingVelocity = velocity;
            pendingTuning   = tuning;
            pendingDecay    = decay;
            pendingDamp     = damp;
            pendingStrike   = strike;
            return;
        }

        applyTrigger (note, velocity, tuning, decay, damp, strike);
    }

    float nextSampleMono (float lcut, float atten)
    {
        // Fire deferred retrigger once fade-out completes
        if (fadeCountdown > 0)
        {
            --fadeCountdown;
            if (fadeCountdown == 0)
                applyTrigger (pendingNote, pendingVelocity, pendingTuning,
                              pendingDecay, pendingDamp, pendingStrike);
        }

        float fc        = std::min (20.0f * std::pow (1000.0f, lcut), sampleRate * 0.499f);
        float lpCoeff   = std::clamp (1.0f - std::exp (-TWO_PI * fc / sampleRate), 0.0f, 1.0f);
        float attenGain = 1.0f - atten * 0.95f;

        float out = 0.0f;
        for (auto& mode : modes)
            out += mode.nextSample();

        out *= attenGain;
        lpState += lpCoeff * (out - lpState);
        return lpState;
    }

private:
    void applyTrigger (int note, float velocity, float tuning,
                       float decay, float damp, float strike)
    {
        float fundamental = 440.0f * std::pow (2.0f, (note - 69.0f + tuning) / 12.0f);
        velocity = std::clamp (velocity, 0.0f, 1.0f);

        float baseDecay = std::pow (1.0f / 1000.0f,
                                    1.0f / (sampleRate * std::max (decay, 0.001f)));

        for (int i = 0; i < NUM_MODES; ++i)
        {
            float ratio = MODE_RATIOS[i];
            float freq  = fundamental * ratio;

            if (freq >= sampleRate * 0.49f)
            {
                modes[i].amplitude = 0.0f;
                continue;
            }

            float falloff   = (1.0f - strike) * 3.0f;
            float amp       = BASE_AMPLITUDES[i] * std::exp (-falloff * i) * velocity;
            float modeDecay = std::pow (baseDecay, 1.0f + damp * ratio);

            modes[i].setup (freq, amp, modeDecay, sampleRate);
        }

        lpState = 0.0f;
    }

    std::array<Mode, NUM_MODES> modes;
    float sampleRate = 44100.0f;
    float lpState    = 0.0f;

    // Retrigger fade state
    int   fadeCountdown   = 0;
    int   pendingNote     = 60;
    float pendingVelocity = 0.0f;
    float pendingTuning   = 0.0f;
    float pendingDecay    = 0.5f;
    float pendingDamp     = 0.3f;
    float pendingStrike   = 0.5f;
};
