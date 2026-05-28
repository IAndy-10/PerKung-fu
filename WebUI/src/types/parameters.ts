// Parameter IDs exposed by the PerKung-fu JUCE plugin (match C++ `ParameterIDs.h`).
export type ParameterId =
    | 'tuning'
    | 'decay'
    | 'damp'
    | 'strike'
    | 'atten'
    | 'lcut'
    | 'mic_gain'
    | 'out_gain'
    | 'threshold';

export interface PluginParameter {
    id: ParameterId;
    name: string;
    value: number; // normalized 0-1 (matches JUCE param->getValue())
    min: number;
    max: number;
    defaultValue: number;
}

export type ParameterState = Record<ParameterId, PluginParameter>;
