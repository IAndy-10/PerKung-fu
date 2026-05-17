#include "PluginProcessor.h"
#include "PluginEditor.h"

PerKungFuProcessor::PerKungFuProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PerKungFuProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::Tuning, "Tuning",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
        0.0f, juce::AudioParameterFloatAttributes{}.withLabel ("st")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::Decay, "Decay",
        juce::NormalisableRange<float> (0.05f, 4.0f, 0.0f, 0.5f),
        0.5f, juce::AudioParameterFloatAttributes{}.withLabel ("s")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::Damp, "Damp",
        juce::NormalisableRange<float> (0.0f, 1.0f),
        0.3f, juce::AudioParameterFloatAttributes{}.withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::Strike, "Strike",
        juce::NormalisableRange<float> (0.0f, 1.0f),
        0.5f, juce::AudioParameterFloatAttributes{}.withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::Atten, "Attenuation",
        juce::NormalisableRange<float> (0.0f, 1.0f),
        0.0f, juce::AudioParameterFloatAttributes{}.withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::LCut, "L/Cut",
        juce::NormalisableRange<float> (0.0f, 1.0f),
        1.0f, juce::AudioParameterFloatAttributes{}.withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::MicGain, "Mic Gain",
        juce::NormalisableRange<float> (0.0f, 2.0f),
        1.0f, juce::AudioParameterFloatAttributes{}.withLabel ("x")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::OutGain, "Out Gain",
        juce::NormalisableRange<float> (-36.0f, 0.0f, 0.0f, 2.5f),
        -1.5f, juce::AudioParameterFloatAttributes{}.withLabel (" dB")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        ParamID::Threshold, "Threshold",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.0f, 2.5f),
        -40.0f, juce::AudioParameterFloatAttributes{}.withLabel (" dB")));

    return layout;
}

void PerKungFuProcessor::prepareToPlay (double sampleRate, int)
{
    voice.setSampleRate (static_cast<float> (sampleRate));
    onset.reset();
}

void PerKungFuProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto tuning      = apvts.getRawParameterValue (ParamID::Tuning.getParamID())->load();
    auto decay       = apvts.getRawParameterValue (ParamID::Decay.getParamID())->load();
    auto damp        = apvts.getRawParameterValue (ParamID::Damp.getParamID())->load();
    auto strike      = apvts.getRawParameterValue (ParamID::Strike.getParamID())->load();
    auto atten       = apvts.getRawParameterValue (ParamID::Atten.getParamID())->load();
    auto lcut        = apvts.getRawParameterValue (ParamID::LCut.getParamID())->load();
    auto micGain     = apvts.getRawParameterValue (ParamID::MicGain.getParamID())->load();
    auto outGainDb   = apvts.getRawParameterValue (ParamID::OutGain.getParamID())->load();
    auto thresholdDb = apvts.getRawParameterValue (ParamID::Threshold.getParamID())->load();

    float outGain      = juce::Decibels::decibelsToGain (outGainDb);
    float thresholdLin = juce::Decibels::decibelsToGain (thresholdDb);
    float sr           = static_cast<float> (getSampleRate());

    int numSamples = buffer.getNumSamples();
    int numIn      = getTotalNumInputChannels();
    int numOut     = getTotalNumOutputChannels();

    // Copy input before clearing (host uses in-place buffers)
    juce::AudioBuffer<float> inputCopy (numIn, numSamples);
    for (int ch = 0; ch < numIn; ++ch)
        inputCopy.copyFrom (ch, 0, buffer.getReadPointer (ch), numSamples);

    // Peak level for the GUI meter
    float peak = 0.0f;
    for (int ch = 0; ch < numIn; ++ch)
        peak = std::max (peak, inputCopy.getMagnitude (ch, 0, numSamples));
    inputLevel.store (peak, std::memory_order_relaxed);

    // Clear output — synthesis replaces the input signal
    for (int ch = 0; ch < numOut; ++ch)
        buffer.clear (ch, 0, numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        // 1. Contact mic onset detection
        float sig = 0.0f;
        for (int ch = 0; ch < numIn; ++ch)
            sig = std::max (sig, std::abs (inputCopy.getSample (ch, i)));
        sig *= micGain;

        float vel = onset.process (sig, thresholdLin, sr);
        if (vel >= 0.0f)
            voice.trigger (60, vel, tuning, decay, damp, strike);

        // 2. Synthesize
        float s = voice.nextSampleMono (lcut, atten) * outGain;
        for (int ch = 0; ch < numOut; ++ch)
            buffer.setSample (ch, i, s);
    }
}

void PerKungFuProcessor::getStateInformation (juce::MemoryBlock& data)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, data);
}

void PerKungFuProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* PerKungFuProcessor::createEditor()
{
    return new PerKungFuEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PerKungFuProcessor();
}
