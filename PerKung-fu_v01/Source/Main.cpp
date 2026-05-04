#include <JuceHeader.h>
#include "MainComponent.h"

class BasicMicEffectApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "Basic Mic Effect"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override           { return false; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override { mainWindow.reset(); }

    void systemRequestedQuit() override { quit(); }

    //==========================================================================
    struct MainWindow : public juce::DocumentWindow
    {
        MainWindow(const juce::String& name)
            : DocumentWindow(name,
                             juce::Colour(0xff1e1e2e),
                             DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            setResizable(false, false);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(BasicMicEffectApp)
