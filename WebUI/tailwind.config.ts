import type { Config } from 'tailwindcss';

const config: Config = {
    content: [
        "./index.html",
        "./src/**/*.{js,ts,jsx,tsx,svelte}",
    ],
    theme: {
        extend: {
            colors: {
                plugin: {
                    bg: '#1e1e1e',
                    panel: '#2a2a2a',
                    accent: '#4a9eff',
                    accentHover: '#6ab4ff',
                    text: '#ffffff',
                    textMuted: '#999999',
                    border: '#3a3a3a',
                    knob: '#353535'
                }
            }
        },
    },
    plugins: [],
};

export default config;
