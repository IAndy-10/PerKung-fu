#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PerKungFuEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit PerKungFuEditor (PerKungFuProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override
    {
        float target = processor.inputLevel.load (std::memory_order_relaxed);
        if (target >= meterLevel)
            meterLevel = target;
        else
            meterLevel *= 0.85f;
        repaint();
    }

    PerKungFuProcessor& processor;
    float meterLevel = 0.0f;

    struct ParamRow
    {
        juce::Label  label;
        juce::Slider slider { juce::Slider::LinearBar, juce::Slider::NoTextBox };
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    std::array<ParamRow, 9> rows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerKungFuEditor)
};
