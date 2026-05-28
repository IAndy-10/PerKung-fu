import './index.css';
import './app.css';
import App from './App.svelte';
import './bridge/bridge';

const target = document.getElementById('app');

if (!target) {
    console.error('[Plugin] Fatal: #app container not found');
} else {
    try {
        new App({ target });
        console.log('[Plugin] Svelte UI initialized');
    } catch (err) {
        console.error('[Plugin] Fatal error during mount:', err);
        target.innerHTML = `<div style="color:#c44;font-family:monospace;padding:20px;">[FATAL ERROR]: UI_MOUNT_FAILED</div>`;
    }
}
