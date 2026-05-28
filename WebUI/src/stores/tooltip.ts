import { writable } from 'svelte/store';

export interface TooltipState {
    visible: boolean;
    x: number;
    y: number;
    name: string;
    value: string;
    range: string;
}

export const tooltipStore = writable<TooltipState>({
    visible: false,
    x: 0, y: 0,
    name: '', value: '', range: '',
});
