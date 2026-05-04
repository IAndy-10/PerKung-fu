#pragma once
#include <JuceHeader.h>
#include "DSP/DrumVoice.h"

// ---------------------------------------------------------------------------
// MainComponent
//
// Real-time contact-mic percussion trigger:
//   1. Runs an envelope follower on the raw mic / contact-mic signal
//   2. When the envelope crosses MIC GATE threshold → triggers DrumVoice
//      (modal circular-membrane synthesis, 8 Bessel-zero modes)
//   3. Output = drum voice only (stereo)
// ---------------------------------------------------------------------------
class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // -----------------------------------------------------------------------
    // Onset detector  (envelope follower + threshold)
    // -----------------------------------------------------------------------
    DrumVoice drumVoice;
    float envFollower    = 0.0f;
    float attackCoeff    = 0.0f;
    float releaseCoeff   = 0.0f;
    bool  wasAboveThresh = false;
    int   retrigCooldown = 0;
    double currentSampleRate = 44100.0;

    // -----------------------------------------------------------------------
    // GUI
    // -----------------------------------------------------------------------
    juce::Slider micGateSlider;
    juce::Label  micGateLabel;
    juce::Slider tuningSlider;
    juce::Label  tuningLabel;
    juce::Slider decaySlider;
    juce::Label  decayLabel;
    juce::Slider dampSlider;
    juce::Label  dampLabel;
    juce::Slider strikeSlider;
    juce::Label  strikeLabel;

    juce::TextButton audioSetupBtn { "Audio Setup..." };

    void styleSlider(juce::Slider& s, juce::Label& l, const juce::String& text,
                     double min, double max, double def, double step = 0.01);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
