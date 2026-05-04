#pragma once
#include <JuceHeader.h>
#include "DrumVoice.h"
#include "ParameterIDs.h"

// Detects transient onsets from a contact mic signal.
// Features: HPF to reject DC/low-freq rumble, 3ms peak window,
// threshold comparison, 50ms refractory cooldown.
struct OnsetDetector
{
    int   cooldown   = 0;
    float hpState    = 0.0f;   // HPF memory
    float peakHold   = 0.0f;   // running peak in current window
    int   windowSamp = 0;      // samples counted in window
    int   windowSize = 0;      // 3ms window size in samples

    void reset()
    {
        cooldown   = 0;
        hpState    = 0.0f;
        peakHold   = 0.0f;
        windowSamp = 0;
        windowSize = 0;
    }

    // Returns velocity [0,1] when an onset is detected, else -1.
    // Call once per sample. sampleRate must be passed every block.
    float process (float sample, float thresholdLin, float sampleRate)
    {
        // Update window size lazily (handles SR changes)
        int newWin = static_cast<int> (sampleRate * 0.003f); // 3 ms
        if (newWin != windowSize)
        {
            windowSize = newWin;
            windowSamp = 0;
            peakHold   = 0.0f;
        }

        // 1. High-pass filter ~100 Hz via first-difference (y = x[n] - x[n-1])
        //    Effective fc ≈ sr / (2π·tau) which removes DC and sub-100Hz rumble.
        float hpOut = sample - hpState;
        hpState = sample;

        // 2. Accumulate peak over window
        float absHp = std::abs (hpOut);
        peakHold = std::max (peakHold, absHp);
        ++windowSamp;

        if (windowSamp < windowSize)
            return -1.0f;

        // Window complete — evaluate
        float peak = peakHold;
        peakHold   = 0.0f;
        windowSamp = 0;

        if (cooldown > 0)
        {
            cooldown -= windowSize;
            if (cooldown < 0) cooldown = 0;
            return -1.0f;
        }

        if (peak >= thresholdLin)
        {
            cooldown = static_cast<int> (sampleRate * 0.05f); // 50 ms refractory
            return juce::jlimit (0.0f, 1.0f, peak);
        }
        return -1.0f;
    }
};

class HapticPercProcessor : public juce::AudioProcessor
{
public:
    HapticPercProcessor();

    // AudioProcessor interface
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Haptic Perc"; }
    bool   acceptsMidi()  const override { return true; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& data) override;
    void setStateInformation (const void* data, int size) override;

    juce::AudioProcessorValueTreeState apvts;

    // Input level meter — written by audio thread, read by GUI timer
    std::atomic<float> inputLevel { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    DrumVoice     voice;
    OnsetDetector onset;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HapticPercProcessor)
};
