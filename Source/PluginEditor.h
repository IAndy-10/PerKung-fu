#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WebviewBridge.h"

class PerKungFuEditor : public juce::AudioProcessorEditor,
                        private juce::Timer
{
public:
    explicit PerKungFuEditor (PerKungFuProcessor&);
    ~PerKungFuEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    PerKungFuProcessor& processorRef;
    std::unique_ptr<WebViewBridge> webView;

    void onParameterChangedFromJS (const juce::String& paramId, float value);
    void sendAllParamsToJS();
    void sendParamToJS (const juce::String& paramId, float normalizedValue);

    std::unordered_map<std::string, float> lastParamValues;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerKungFuEditor)
};
