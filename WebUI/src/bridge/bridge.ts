import { setParameterValue as updateStore, inputLevel, outputLevel } from '../state/store';
import type { ParameterId } from '../types/parameters';

// Bidirectional bridge between C++ (JUCE) and TypeScript/Svelte frontend.
// C++ → JS: window.setParameterValue(id, normalizedValue)
// JS → C++: window.location.href = "juce://setparameter?name=id&value=normalized"
class ParameterBridge {
    constructor() {
        this.initGlobalCallbacks();
    }

    private initGlobalCallbacks() {
        // Called by C++ via evaluateJavascript()
        (window as any).setParameterValue = (id: ParameterId, value: number) => {
            updateStore(id, value);
        };

        (window as any).onBackendMessage = (message: any) => {
            if (message?.type === 'setParameterValue') {
                updateStore(message.parameterId, message.value);
            }
        };

        // Called by C++ timerCallback at 30 Hz — raw block peak linear amplitude
        (window as any).setInputLevel  = (v: number) => { inputLevel.set(v);  };
        (window as any).setOutputLevel = (v: number) => { outputLevel.set(v); };
    }

    // Send normalized 0-1 value to C++ APVTS via juce:// URL scheme
    sendParameterChange(id: ParameterId, value: number) {
        if (window.location.protocol.startsWith('http')) return;
        window.location.href = `juce://setparameter?name=${id}&value=${value}`;
    }
}

export const bridge = new ParameterBridge();
