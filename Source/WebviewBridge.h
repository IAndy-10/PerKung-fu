#pragma once

#include <JuceHeader.h>
#include <functional>

// Bridge between the WebView (TypeScript/Svelte UI) and C++ plugin code.
// JS → C++: Intercepts juce:// URLs for parameter changes.
// C++ → JS: Uses evaluateJavascript() to call window.setParameterValue().
class WebViewBridge : public juce::WebBrowserComponent
{
public:
    WebViewBridge() = default;

    using ParameterCallback  = std::function<void (const juce::String&, float)>;
    using PageLoadedCallback = std::function<void()>;

    void setParameterCallback  (ParameterCallback  cb) { parameterCallback  = std::move (cb); }
    void setPageLoadedCallback (PageLoadedCallback cb) { pageLoadedCallback = std::move (cb); }

    bool pageAboutToLoad (const juce::String& newURL) override
    {
        if (newURL.startsWith ("juce://"))
        {
            handleBridgeURL (newURL);
            return false;
        }
        return true;
    }

    void pageFinishedLoading (const juce::String& url) override
    {
        // Ignore the synthetic juce:// intercepts; only react to real page loads.
        if (pageLoadedCallback && ! url.startsWith ("juce://"))
            pageLoadedCallback();
    }

private:
    ParameterCallback  parameterCallback;
    PageLoadedCallback pageLoadedCallback;

    void handleBridgeURL (const juce::String& urlStr)
    {
        if (! urlStr.contains ("setparameter")) return;

        juce::URL juceURL (urlStr);
        auto paramNames  = juceURL.getParameterNames();
        auto paramValues = juceURL.getParameterValues();

        int nameIdx  = paramNames.indexOf ("name");
        int valueIdx = paramNames.indexOf ("value");

        if (nameIdx >= 0 && valueIdx >= 0 && parameterCallback)
        {
            juce::String paramName = paramValues[nameIdx];
            float paramValue       = paramValues[valueIdx].getFloatValue();
            parameterCallback (paramName, paramValue);
        }
    }
};
