import { writable } from 'svelte/store';

export interface NormalizedLandmark {
    x: number;
    y: number;
    z: number;
}

export interface HandDetection {
    landmarks:    NormalizedLandmark[][];
    handednesses: { categoryName: string; score: number }[][];
}

// Written by GestureController after each detectForVideo() call.
// Read by Camera to draw the skeleton overlay.
export const latestHands = writable<HandDetection | null>(null);
