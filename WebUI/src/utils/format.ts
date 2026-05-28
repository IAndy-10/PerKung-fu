/** Central value-formatting utilities. */

/** Format a numeric value with optional unit.
 *  Special rule: if unit === ' ms' and value >= 1000, show zero decimal places. */
export function formatValue(value: number, unit: string, decimals: number): string {
    if (unit === ' ms' && value >= 1000) return value.toFixed(0) + unit;
    return value.toFixed(decimals) + unit;
}

/** Adaptive time display: below 1000 ms → "XXX ms", at or above → "X.XX s". */
export function formatTime(ms: number): string {
    if (ms < 1000) return `${Math.round(ms)} ms`;
    return `${(ms / 1000).toFixed(2)} s`;
}
