#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class HapticPercEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit HapticPercEditor (HapticPercProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override
    {
        float target = processor.inputLevel.load (std::memory_order_relaxed);
        if (target >= meterLevel)
            meterLevel = target;                    // instant attack
        else
            meterLevel *= 0.85f;                    // ~10 dB/s decay at 30 Hz
        repaint();
    }

    HapticPercProcessor& processor;
    float meterLevel = 0.0f;

    struct ParamRow
    {
        juce::Label  label;
        juce::Slider slider { juce::Slider::LinearBar, juce::Slider::NoTextBox };
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    std::array<ParamRow, 9> rows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HapticPercEditor)
};
