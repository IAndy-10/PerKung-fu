import { defineConfig } from 'vite';
import { viteSingleFile } from 'vite-plugin-singlefile';
import { svelte, vitePreprocess } from '@sveltejs/vite-plugin-svelte';

export default defineConfig({
    plugins: [
        ...(process.env.NODE_ENV === 'production' ? [viteSingleFile()] : []),
        svelte({
            preprocess: vitePreprocess(),
        }),
    ],
    build: {
        outDir: 'dist',
        emptyOutDir: true,
    },
    server: {
        middlewareMode: false,
    },
});