// Modal synthesis DSP engine for physical drum modelling.
//
// Based on digital waveguide theory from:
// "Physical Modeling of Percussion Instruments" - Anders Gärder, Chalmers 2005
//
// The circular membrane modes are approximated using their Bessel function zero
// frequency ratios. Each mode is a damped sinusoidal resonator.

use std::f32::consts::TAU;

const NUM_MODES: usize = 8;

// Frequency ratios of circular membrane modes, normalised to the fundamental (0,1).
// Derived from Bessel function zeros J_m divided by the first zero of J_0 (2.4048).
// See Chalmers paper table 3.1 and section 3.3.4.
const MODE_RATIOS: [f32; NUM_MODES] = [
    1.000, // (0,1) fundamental  - J_0 zero: 2.4048
    1.593, // (1,1)              - J_1 zero: 3.8317
    2.135, // (2,1)              - J_2 zero: 5.1356
    2.295, // (0,2)              - J_0 zero: 5.5201
    2.653, // (3,1)              - J_3 zero: 6.3802
    2.917, // (1,2)              - J_1 zero: 7.0156
    3.156, // (4,1)              - J_4 zero: 7.5883
    3.500, // (2,2)              - J_2 zero: 8.4172
];

// Relative amplitude weights per mode.
// Higher modes carry less energy in a typical drum strike.
const BASE_AMPLITUDES: [f32; NUM_MODES] = [
    1.00, 0.75, 0.55, 0.45, 0.35, 0.28, 0.22, 0.18,
];

// A single sinusoidal resonator representing one drum membrane mode.
struct Mode {
    phase: f32,
    amplitude: f32,
    phase_inc: f32,
    decay: f32,
}

impl Mode {
    const fn new() -> Self {
        Self {
            phase: 0.0,
            amplitude: 0.0,
            phase_inc: 0.0,
            decay: 0.999,
        }
    }

    fn setup(&mut self, freq: f32, amp: f32, decay: f32, sample_rate: f32) {
        self.phase = 0.0;
        self.amplitude = amp;
        self.phase_inc = TAU * freq / sample_rate;
        self.decay = decay;
    }

    fn next_sample(&mut self) -> f32 {
        if self.amplitude.abs() < 1e-7 {
            return 0.0;
        }
        let s = self.amplitude * self.phase.sin();
        self.phase += self.phase_inc;
        if self.phase >= TAU {
            self.phase -= TAU;
        }
        self.amplitude *= self.decay;
        s
    }
}

// A single drum voice: a bank of modal resonators triggered by a note-on event.
pub struct DrumVoice {
    modes: [Mode; NUM_MODES],
    sample_rate: f32,
    active: bool,
    // One-pole low-pass filter state (shared across all modes)
    lp_state: f32,
}

impl DrumVoice {
    pub fn new(sample_rate: f32) -> Self {
        Self {
            modes: [
                Mode::new(),
                Mode::new(),
                Mode::new(),
                Mode::new(),
                Mode::new(),
                Mode::new(),
                Mode::new(),
                Mode::new(),
            ],
            sample_rate,
            active: false,
            lp_state: 0.0,
        }
    }

    /// Trigger the drum voice.
    ///
    /// - `note_key`: MIDI note number (0-127), sets the fundamental pitch.
    /// - `velocity`: MIDI velocity (0.0-1.0), scales the output amplitude.
    /// - `tuning`: Semitone offset applied to the MIDI note (±24 st range).
    /// - `decay`: T60 time in seconds (time to reach -60 dB).
    /// - `damp`: Frequency-dependent damping [0,1]. Higher values make upper
    ///   modes decay faster, emulating membrane damping (eq. 3.19, Chalmers).
    /// - `strike`: Strike hardness [0,1]. Hard strike excites all modes equally;
    ///   soft strike concentrates energy in the lower modes.
    pub fn trigger(
        &mut self,
        note_key: u8,
        velocity: f64,
        tuning: f32,
        decay: f32,
        damp: f32,
        strike: f32,
    ) {
        // Fundamental frequency from MIDI note + tuning offset.
        let fundamental = 440.0 * 2.0_f32.powf((note_key as f32 - 69.0 + tuning) / 12.0);
        let velocity_f = (velocity as f32).clamp(0.0, 1.0);

        // Per-sample base decay coefficient from equation 3.19 (Chalmers):
        //   λ = (1/1000)^(1 / (Fs * T60))
        // This gives -60 dB attenuation after T60 seconds.
        let t60 = decay.max(0.001);
        let base_decay = (1.0_f32 / 1000.0_f32).powf(1.0 / (self.sample_rate * t60));

        for (i, mode) in self.modes.iter_mut().enumerate() {
            let ratio = MODE_RATIOS[i];
            let freq = fundamental * ratio;

            // Silence modes above Nyquist to avoid aliasing.
            if freq >= self.sample_rate * 0.49 {
                mode.amplitude = 0.0;
                mode.phase_inc = 0.0;
                mode.decay = 1.0;
                continue;
            }

            // Strike hardness controls the spectral rolloff across modes.
            // Hard strike (1.0): flat spectrum across all modes.
            // Soft strike (0.0): steep exponential rolloff, only low modes.
            let falloff = (1.0 - strike) * 3.0;
            let mode_amp = BASE_AMPLITUDES[i] * (-falloff * i as f32).exp() * velocity_f;

            // Frequency-dependent decay (DAMP parameter).
            // From Chalmers section 3.4.1: the per-sample damping factor is
            // raised to a power proportional to the mode's frequency ratio,
            // so higher modes decay faster when damp > 0.
            let mode_decay = base_decay.powf(1.0 + damp * ratio);

            mode.setup(freq, mode_amp, mode_decay, self.sample_rate);
        }

        self.lp_state = 0.0;
        self.active = true;
    }

    pub fn is_active(&self) -> bool {
        self.active
    }

    /// Generate the next block of samples, adding the output into `buffer`.
    ///
    /// - `lcut`: Low-pass cutoff [0,1], mapped exponentially to 20 Hz–20 kHz.
    /// - `atten`: Output attenuation [0,1] (0 = full resonance, 1 = muted).
    pub fn generate_samples(&mut self, buffer: &mut [f32], lcut: f32, atten: f32) {
        // Map lcut [0,1] → fc [20, 20000] Hz using exponential scaling.
        let fc = (20.0_f32 * 1000.0_f32.powf(lcut)).min(self.sample_rate * 0.499);

        // One-pole LP coefficient: α = 1 - exp(-2π·fc/Fs)
        let lp_coeff = (1.0 - (-TAU * fc / self.sample_rate).exp()).clamp(0.0, 1.0);

        // ATTEN reduces the resonator output level.
        let atten_gain = 1.0 - atten * 0.95;

        for sample in buffer.iter_mut() {
            let mut out = 0.0_f32;
            for mode in self.modes.iter_mut() {
                out += mode.next_sample();
            }
            out *= atten_gain;

            // One-pole low-pass filter.
            self.lp_state += lp_coeff * (out - self.lp_state);
            *sample += self.lp_state;
        }

        // Mark voice inactive when all resonators have decayed below threshold.
        self.active = self.modes.iter().any(|m| m.amplitude.abs() > 1e-6);
    }
}
