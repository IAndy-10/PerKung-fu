import { setParameterValue as updateStore, inputLevel, outputLevel } from '../state/store';
import type { ParameterId } from '../types/parameters';

// Bidirectional bridge between C++ (JUCE) and TypeScript/Svelte frontend.
// C++ → JS: window.setParameterValue(id, normalizedValue)
// JS → C++: window.location.href = "juce://setparameter?name=id&value=normalized"
class ParameterBridge {
    // Queue for outgoing parameter messages — only one juce:// navigation can
    // be in-flight at a time; the browser cancels pending navigations if you
    // assign window.location.href again before the first one is processed.
    private queue: Array<{ id: string; value: number }> = [];
    private sending = false;

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

    // Send normalized 0-1 value to C++ APVTS via juce:// URL scheme.
    // Enqueues the message so rapid calls (e.g. preset application) don't
    // cancel each other — WebKit only processes one location.href at a time.
    sendParameterChange(id: ParameterId, value: number) {
        if (window.location.protocol.startsWith('http')) return;
        this.queue.push({ id, value });
        if (!this.sending) this.flush();
    }

    private flush() {
        if (this.queue.length === 0) { this.sending = false; return; }
        this.sending = true;
        const { id, value } = this.queue.shift()!;
        console.log(`[bridge] → ${id}=${value}  (${this.queue.length} remaining)`);
        window.location.href = `juce://setparameter?name=${id}&value=${value}`;
        // Yield to WebKit so pageAboutToLoad fires before the next message.
        // 16 ms gives the WKWebView IPC round-trip time to complete reliably.
        setTimeout(() => this.flush(), 16);
    }
}

export const bridge = new ParameterBridge();
