#include "PluginEditor.h"
#include "ParameterIDs.h"

static const std::pair<juce::ParameterID, const char*> PARAM_ROWS[] = {
    { ParamID::Tuning,     "Tuning"      },
    { ParamID::Decay,      "Decay"       },
    { ParamID::Damp,       "Damp"        },
    { ParamID::Strike,     "Strike"      },
    { ParamID::Atten,      "Attenuation" },
    { ParamID::LCut,       "L/Cut"       },
    { ParamID::MicGain,    "Mic Gain"    },
    { ParamID::OutGain,    "Out Gain"    },
    { ParamID::Threshold,  "Threshold"   },
};

HapticPercEditor::HapticPercEditor (HapticPercProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    for (int i = 0; i < 9; ++i)
    {
        auto& row = rows[(size_t) i];
        auto& [paramId, name] = PARAM_ROWS[i];

        row.label.setText (name, juce::dontSendNotification);
        row.label.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (row.label);

        row.slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
        addAndMakeVisible (row.slider);

        row.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, paramId.getParamID(), row.slider);
    }

    setSize (400, 394);
    startTimerHz (30);
}

void HapticPercEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e2e));

    // Title
    g.setColour (juce::Colour (0xffcdd6f4));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("Haptic Perc", getLocalBounds().removeFromTop (36),
                juce::Justification::centred, false);

    // Separator above threshold row
    auto sepY = 36 + 12 + 8 * (28 + 4) - 2;
    g.setColour (juce::Colour (0xff45475a));
    g.drawHorizontalLine (sepY, 12.0f, (float) getWidth() - 12.0f);

    // Input level meter at the bottom (uses ballistics-smoothed meterLevel)
    auto meterArea = getLocalBounds().removeFromBottom (14).reduced (12, 2);
    g.setColour (juce::Colour (0xff313244));
    g.fillRect (meterArea);
    g.setColour (meterLevel > 0.01f ? juce::Colour (0xffa6e3a1) : juce::Colour (0xff45475a));
    g.fillRect (meterArea.withWidth ((int) (meterArea.getWidth() * juce::jlimit (0.0f, 1.0f, meterLevel))));
    g.setColour (juce::Colour (0xff6c7086));
    g.setFont (juce::FontOptions (10.0f));
    g.drawText ("IN", meterArea, juce::Justification::centredLeft, false);
}

void HapticPercEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    area.removeFromBottom (14); // meter
    area.removeFromTop (36);    // title

    const int rowH   = 28;
    const int labelW = 90;
    const int gap    = 4;

    for (auto& row : rows)
    {
        auto rowArea = area.removeFromTop (rowH);
        area.removeFromTop (gap);

        row.label.setBounds (rowArea.removeFromLeft (labelW));
        row.slider.setBounds (rowArea);
    }
}
