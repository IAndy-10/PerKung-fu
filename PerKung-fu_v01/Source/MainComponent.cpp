#include "MainComponent.h"

static const juce::Colour kBg     { 0xff1e1e2e };
static const juce::Colour kPanel  { 0xff181825 };
static const juce::Colour kAccent { 0xff89b4fa };
static const juce::Colour kText   { 0xffcdd6f4 };
static const juce::Colour kMuted  { 0xff6c7086 };
static const juce::Colour kTrack  { 0xff313244 };

//==============================================================================
MainComponent::MainComponent()
{
    styleSlider(micGateSlider, micGateLabel, "MIC GATE  (threshold)", 0.0,   1.0,  0.05, 0.001);
    styleSlider(tuningSlider,  tuningLabel,  "TUNING  (semitones)",  -12.0, 12.0,  0.0,  0.1);
    styleSlider(decaySlider,   decayLabel,   "DECAY  (seconds)",      0.05,  4.0,  0.5,  0.01);
    styleSlider(dampSlider,    dampLabel,    "DAMP",                  0.0,   1.0,  0.3,  0.01);
    styleSlider(strikeSlider,  strikeLabel,  "STRIKE",                0.0,   1.0,  0.5,  0.01);

    audioSetupBtn.setColour(juce::TextButton::buttonColourId,  kPanel);
    audioSetupBtn.setColour(juce::TextButton::textColourOffId, kText);
    audioSetupBtn.onClick = [this]
    {
        juce::DialogWindow::LaunchOptions opts;
        auto* sel = new juce::AudioDeviceSelectorComponent(
            deviceManager, 1, 2, 2, 2, false, false, true, false);
        sel->setSize(500, 320);
        opts.content.setOwned(sel);
        opts.dialogTitle                  = "Audio Setup";
        opts.dialogBackgroundColour       = kBg;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar            = true;
        opts.resizable                    = false;
        opts.launchAsync();
    };
    addAndMakeVisible(audioSetupBtn);

    setSize(460, 370);
    setAudioChannels(2, 2);  // 2-in: works with both built-in (mono) and Focusrite 2i2
}

MainComponent::~MainComponent() { shutdownAudio(); }

//==============================================================================
void MainComponent::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
    currentSampleRate = sampleRate;
    drumVoice.setSampleRate(static_cast<float>(sampleRate));

    attackCoeff  = std::exp(-1.0f / (0.001f * static_cast<float>(sampleRate)));
    releaseCoeff = std::exp(-1.0f / (0.080f * static_cast<float>(sampleRate)));
    envFollower  = 0.0f;

    wasAboveThresh = false;
    retrigCooldown = 0;
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    juce::ScopedNoDenormals noDenormals;

    auto* buffer     = bufferToFill.buffer;
    const int start  = bufferToFill.startSample;
    const int nSamps = bufferToFill.numSamples;

    // Sum all input channels to mono so the contact mic works on Input 1 or Input 2
    const int numIns = std::min(buffer->getNumChannels(), 2);
    const float* inPtr0 = buffer->getReadPointer(0, start);
    const float* inPtr1 = numIns > 1 ? buffer->getReadPointer(1, start) : nullptr;

    const float threshold = static_cast<float>(micGateSlider.getValue());
    const float tuning    = static_cast<float>(tuningSlider.getValue());
    const float decay     = static_cast<float>(decaySlider.getValue());
    const float damp      = static_cast<float>(dampSlider.getValue());
    const float strike    = static_cast<float>(strikeSlider.getValue());

    const int kCooldownSamples = static_cast<int>(currentSampleRate * 0.04);

    for (int i = 0; i < nSamps; ++i)
    {
        // 1. Envelope follower — sum both inputs to mono (covers Input 1 and Input 2)
        const float micSample = inPtr0[i] + (inPtr1 ? inPtr1[i] : 0.0f);
        const float level = std::abs(micSample);
        if (level > envFollower)
            envFollower = attackCoeff * envFollower + (1.0f - attackCoeff) * level;
        else
            envFollower = releaseCoeff * envFollower;

        // 2. Onset detection → trigger DrumVoice
        if (threshold > 1e-4f)
        {
            if (!wasAboveThresh && envFollower >= threshold && retrigCooldown == 0)
            {
                const float vel = std::min(envFollower / threshold, 1.0f);
                drumVoice.trigger(60, vel, tuning, decay, damp, strike);
                retrigCooldown = kCooldownSamples;
                wasAboveThresh = true;
            }
            else if (envFollower < threshold * 0.5f)
            {
                wasAboveThresh = false;
            }
        }

        if (retrigCooldown > 0) --retrigCooldown;

        // 3. Drum voice only
        const float out = drumVoice.nextSampleMono(1.0f, 0.0f) * DRUM_OUTPUT_SCALE;
        buffer->setSample(0, start + i, out);
        if (buffer->getNumChannels() > 1)
            buffer->setSample(1, start + i, out);
    }
}

void MainComponent::releaseResources() {}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(kBg);

    g.setColour(kPanel);
    g.fillRect(0, 0, getWidth(), 44);
    g.setColour(kAccent);
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(15.0f).withStyle("Bold")));
    g.drawText("BASIC MIC EFFECT  //  Contact Mic Perc Trigger",
               0, 0, getWidth(), 44, juce::Justification::centred);

    // Section separator
    g.setColour(kTrack);
    g.fillRect(18, 52, getWidth() - 36, 1);
    g.setColour(kMuted);
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(10.5f).withStyle("Bold")));
    g.drawText("TRIGGER", 18, 55, 200, 12, juce::Justification::centredLeft);

    g.setColour(kTrack);
    g.fillRect(18, 116, getWidth() - 36, 1);
    g.setColour(kMuted);
    g.drawText("DRUM VOICE", 18, 119, 200, 12, juce::Justification::centredLeft);
}

void MainComponent::resized()
{
    const int pad  = 18;
    const int lh   = 16;
    const int sh   = 30;
    const int rowH = lh + sh + 4;
    const int sepH = 20;

    int y = 44 + sepH;

    auto placeRow = [&](juce::Label& lbl, juce::Slider& sld)
    {
        lbl.setBounds(pad, y, getWidth() - pad * 2, lh);
        sld.setBounds(pad, y + lh, getWidth() - pad * 2, sh);
        y += rowH;
    };

    placeRow(micGateLabel, micGateSlider);

    y += sepH;
    placeRow(tuningLabel,  tuningSlider);
    placeRow(decayLabel,   decaySlider);
    placeRow(dampLabel,    dampSlider);
    placeRow(strikeLabel,  strikeSlider);

    y += 8;
    audioSetupBtn.setBounds(getWidth() - pad - 110, y, 110, 26);
}

//==============================================================================
void MainComponent::styleSlider(juce::Slider& s, juce::Label& l, const juce::String& text,
                                double min, double max, double def, double step)
{
    l.setText(text, juce::dontSendNotification);
    l.setFont(juce::Font(juce::FontOptions{}.withHeight(12.5f)));
    l.setColour(juce::Label::textColourId, kText);
    addAndMakeVisible(l);

    s.setRange(min, max, step);
    s.setValue(def, juce::dontSendNotification);
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 55, 20);
    s.setColour(juce::Slider::thumbColourId,          kAccent);
    s.setColour(juce::Slider::trackColourId,          kTrack);
    s.setColour(juce::Slider::backgroundColourId,     kPanel);
    s.setColour(juce::Slider::textBoxTextColourId,    kText);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(s);
}
