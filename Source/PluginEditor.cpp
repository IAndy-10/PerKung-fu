#include "PluginEditor.h"
#include "ParameterIDs.h"

PerKungFuEditor::PerKungFuEditor (PerKungFuProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    webView = std::make_unique<WebViewBridge>();
    addAndMakeVisible (webView.get());

    // JS → C++: parameter changes from the UI
    webView->setParameterCallback ([this] (const juce::String& paramId, float value) {
        onParameterChangedFromJS (paramId, value);
    });

    // Page loaded: push all current param values now that JS is ready.
    // Clear lastParamValues so the timer also detects them as "new" in case
    // any value changes between page load and the first timer tick.
    webView->setPageLoadedCallback ([this]() {
        lastParamValues.clear();
        sendAllParamsToJS();
    });

    setSize (1200, 760);
    setResizable (true, true);

    // Embed the built WebUI into a temp dir and load it
    auto tempDir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                       .getChildFile ("PerKungFu_" + juce::String (juce::Time::currentTimeMillis()));
    tempDir.createDirectory();

    auto htmlFile = tempDir.getChildFile ("index.html");
    htmlFile.replaceWithData (BinaryData::index_html, BinaryData::index_htmlSize);

    webView->goToURL ("file://" + htmlFile.getFullPathName());

    startTimerHz (30);
}

PerKungFuEditor::~PerKungFuEditor()
{
    stopTimer();
}

void PerKungFuEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0c0a08));
}

void PerKungFuEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void PerKungFuEditor::timerCallback()
{
    auto& apvts = processorRef.apvts;

    // Push any parameter values that have changed since last tick
    auto sendIfChanged = [&] (const juce::String& paramId) {
        auto* param = apvts.getParameter (paramId);
        if (! param) return;
        float normalized = param->getValue();
        auto key = paramId.toStdString();
        auto it  = lastParamValues.find (key);
        if (it == lastParamValues.end() || std::abs (it->second - normalized) > 0.001f)
        {
            lastParamValues[key] = normalized;
            sendParamToJS (paramId, normalized);
        }
    };

    sendIfChanged (ParamID::Tuning.getParamID());
    sendIfChanged (ParamID::Decay.getParamID());
    sendIfChanged (ParamID::Damp.getParamID());
    sendIfChanged (ParamID::Strike.getParamID());
    sendIfChanged (ParamID::Atten.getParamID());
    sendIfChanged (ParamID::LCut.getParamID());
    sendIfChanged (ParamID::MicGain.getParamID());
    sendIfChanged (ParamID::OutGain.getParamID());
    sendIfChanged (ParamID::Threshold.getParamID());

    // Push real audio level meters to JS (raw block peaks, JS applies ballistics)
    float inLvl  = processorRef.inputLevel.load  (std::memory_order_relaxed);
    float outLvl = processorRef.outputLevel.load (std::memory_order_relaxed);

    webView->evaluateJavascript (
        "if(window.setInputLevel){window.setInputLevel(" + juce::String (inLvl, 4) + ");}");
    webView->evaluateJavascript (
        "if(window.setOutputLevel){window.setOutputLevel(" + juce::String (outLvl, 4) + ");}");

    // Push FFT spectrum (40 log-spaced magnitude bins, 0..1)
    juce::String specJs = "if(window.setSpectrum){window.setSpectrum([";
    for (int i = 0; i < PerKungFuProcessor::NUM_SPEC; ++i)
    {
        if (i > 0) specJs += ",";
        specJs += juce::String (processorRef.spectrumData[i].load (std::memory_order_relaxed), 3);
    }
    specJs += "]);}";
    webView->evaluateJavascript (specJs);
}

void PerKungFuEditor::sendAllParamsToJS()
{
    auto& apvts = processorRef.apvts;

    auto sendAll = [&] (const juce::String& paramId) {
        auto* param = apvts.getParameter (paramId);
        if (! param) return;
        float normalized = param->getValue();
        lastParamValues[paramId.toStdString()] = normalized;
        sendParamToJS (paramId, normalized);
    };

    sendAll (ParamID::Tuning.getParamID());
    sendAll (ParamID::Decay.getParamID());
    sendAll (ParamID::Damp.getParamID());
    sendAll (ParamID::Strike.getParamID());
    sendAll (ParamID::Atten.getParamID());
    sendAll (ParamID::LCut.getParamID());
    sendAll (ParamID::MicGain.getParamID());
    sendAll (ParamID::OutGain.getParamID());
    sendAll (ParamID::Threshold.getParamID());
}

void PerKungFuEditor::sendParamToJS (const juce::String& paramId, float normalizedValue)
{
    juce::String js = "if(window.setParameterValue){window.setParameterValue('"
                      + paramId + "'," + juce::String (normalizedValue, 6) + ");}";
    webView->evaluateJavascript (js);
}

void PerKungFuEditor::onParameterChangedFromJS (const juce::String& paramId, float value)
{
    auto* param = processorRef.apvts.getParameter (paramId);
    if (param != nullptr)
    {
        param->setValueNotifyingHost (value);
        lastParamValues[paramId.toStdString()] = value;
    }
}
