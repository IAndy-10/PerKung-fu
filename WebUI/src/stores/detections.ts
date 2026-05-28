import { writable } from 'svelte/store';

export interface Detection {
    label: string;
    score: number;
    // Normalized coordinates (0..1) in raw video frame space
    box: { left: number; top: number; right: number; bottom: number };
}

// Written by ObjectDetector after each detectForVideo() call.
// Read by Camera (bbox overlay) and App (detections panel + presets).
export const detections = writable<Detection[]>([]);
