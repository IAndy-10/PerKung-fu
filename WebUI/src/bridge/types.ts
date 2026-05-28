import type { ParameterId } from '../types/parameters';

// ============================================================
// Messages FROM C++ TO Frontend
// ============================================================

export interface SetParameterValueMessage {
    type: 'setParameterValue';
    parameterId: ParameterId;
    value: number; // 0-1 normalized
}

export interface AudioStateMessage {
    type: 'audioState';
    isPlaying: boolean;
    isBypassed: boolean;
}

export interface ErrorMessage {
    type: 'error';
    severity: 'error' | 'warning' | 'info';
    message: string;
}

export type BackendMessage = SetParameterValueMessage | AudioStateMessage | ErrorMessage;

// ============================================================
// Type Guards
// ============================================================

export function isSetParameterValueMessage(obj: any): obj is SetParameterValueMessage {
    return (
        obj &&
        obj.type === 'setParameterValue' &&
        typeof obj.parameterId === 'string' &&
        typeof obj.value === 'number'
    );
}

export function isBackendMessage(obj: any): obj is BackendMessage {
    if (!obj || typeof obj.type !== 'string') return false;
    switch (obj.type) {
        case 'setParameterValue': return isSetParameterValueMessage(obj);
        case 'audioState': return typeof obj.isPlaying === 'boolean';
        case 'error': return typeof obj.message === 'string';
        default: return false;
    }
}

export function describeMessage(msg: BackendMessage): string {
    switch (msg.type) {
        case 'setParameterValue':
            return `C++ → "${msg.parameterId}" = ${msg.value.toFixed(3)}`;
        case 'audioState':
            return `C++ → audioState playing=${msg.isPlaying}`;
        case 'error':
            return `C++ → error [${msg.severity}]: ${msg.message}`;
    }
}
