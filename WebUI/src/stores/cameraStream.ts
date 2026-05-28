import { writable } from 'svelte/store';

// Shared camera MediaStream — written by Camera.svelte, consumed by GestureController.svelte
export const cameraStream = writable<MediaStream | null>(null);
