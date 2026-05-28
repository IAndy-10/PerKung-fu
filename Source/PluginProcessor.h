#pragma once
#include <JuceHeader.h>
#include "DSP/DrumVoice.h"
#include "ParameterIDs.h"

// Detects transient onsets from a contact mic signal.
// Features: HPF to reject DC/low-freq rumble, 3ms peak window,
// threshold comparison, 50ms refractory cooldown.
struct OnsetDetector
{
    int   cooldown   = 0;
    float hpState    = 0.0f;
    float peakHold   = 0.0f;
    int   windowSamp = 0;
    int   windowSize = 0;

    void reset()
    {
        cooldown   = 0;
        hpState    = 0.0f;
        peakHold   = 0.0f;
        windowSamp = 0;
        windowSize = 0;
    }

    // Returns velocity [0,1] when an onset is detected, else -1.
    float process (float sample, float thresholdLin, float sampleRate)
    {
        int newWin = static_cast<int> (sampleRate * 0.003f); // 3 ms
        if (newWin != windowSize)
        {
            windowSize = newWin;
            windowSamp = 0;
            peakHold   = 0.0f;
        }

        // High-pass via first-difference — removes DC and sub-100 Hz rumble
        float hpOut = sample - hpState;
        hpState = sample;

        float absHp = std::abs (hpOut);
        peakHold = std::max (peakHold, absHp);
        ++windowSamp;

        if (windowSamp < windowSize)
            return -1.0f;

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

class PerKungFuProcessor : public juce::AudioProcessor
{
public:
    PerKungFuProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "PerKung-fu"; }
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

    // Level meters + spectrum — written by audio thread, read by GUI timer
    std::atomic<float> inputLevel  { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

    static constexpr int FFT_ORDER = 10;           // 1024-point FFT
    static constexpr int FFT_SIZE  = 1 << FFT_ORDER;
    static constexpr int NUM_SPEC  = 40;

    std::array<std::atomic<float>, NUM_SPEC> spectrumData {};

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    DrumVoice     voice;
    OnsetDetector onset;

    juce::dsp::FFT fftProcessor { FFT_ORDER };
    juce::dsp::WindowingFunction<float> fftWindow {
        (size_t)FFT_SIZE, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, FFT_SIZE * 2> fftInOut {};
    std::array<float, FFT_SIZE>     fftAccum {};
    int fftPos = 0;

    void updateSpectrum();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerKungFuProcessor)
};
