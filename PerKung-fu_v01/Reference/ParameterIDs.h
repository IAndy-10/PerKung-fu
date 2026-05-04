#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
    inline const juce::ParameterID Tuning  { "tuning",   1 };
    inline const juce::ParameterID Decay   { "decay",    1 };
    inline const juce::ParameterID Damp    { "damp",     1 };
    inline const juce::ParameterID Strike  { "strike",   1 };
    inline const juce::ParameterID Atten   { "atten",    1 };
    inline const juce::ParameterID LCut    { "lcut",     1 };
    inline const juce::ParameterID MicGain   { "mic_gain",  1 };
    inline const juce::ParameterID OutGain   { "out_gain",  1 };
    inline const juce::ParameterID Threshold { "threshold", 1 };
}
