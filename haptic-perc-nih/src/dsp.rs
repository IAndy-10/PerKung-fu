// Modal synthesis DSP engine for physical drum modelling.
//
// Circular membrane modes approximated via Bessel function zero frequency ratios.
// Source: "Physical Modeling of Percussion Instruments" — Anders Gärder, Chalmers 2005
//
// Key equations used:
//   Decay (§3.4.1 eq.3.19): λ = (1/1000)^(1 / (Fs · T60))
//   Freq-dependent damp:    mode_decay = base_decay ^ (1 + damp · freq_ratio)

use std::f32::consts::TAU;

const NUM_MODES: usize = 8;

// Frequency ratios of circular membrane modes, normalised to the fundamental (0,1).
// Derived from Bessel function zeros J_m / 2.4048 (first zero of J_0).
const MODE_RATIOS: [f32; NUM_MODES] = [
    1.000, // (0,1) — J0 zero: 2.4048
    1.593, // (1,1) — J1 zero: 3.8317
    2.135, // (2,1) — J2 zero: 5.1356
    2.295, // (0,2) — J0 zero: 5.5201
    2.653, // (3,1) — J3 zero: 6.3802
    2.917, // (1,2) — J1 zero: 7.0156
    3.156, // (4,1) — J4 zero: 7.5883
    3.500, // (2,2) — J2 zero: 8.4172
];

// Base amplitude weights per mode (higher modes carry less energy).
const BASE_AMPLITUDES: [f32; NUM_MODES] = [
    1.00, 0.75, 0.55, 0.45, 0.35, 0.28, 0.22, 0.18,
];

struct Mode {
    phase: f32,
    amplitude: f32,
    phase_inc: f32,
    decay: f32,
}

impl Mode {
    const fn new() -> Self {
        Self { phase: 0.0, amplitude: 0.0, phase_inc: 0.0, decay: 0.999 }
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

pub struct DrumVoice {
    modes: [Mode; NUM_MODES],
    sample_rate: f32,
    active: bool,
    lp_state: f32,
}

impl DrumVoice {
    pub fn new(sample_rate: f32) -> Self {
        Self {
            modes: [
                Mode::new(), Mode::new(), Mode::new(), Mode::new(),
                Mode::new(), Mode::new(), Mode::new(), Mode::new(),
            ],
            sample_rate,
            active: false,
            lp_state: 0.0,
        }
    }

    /// Trigger the drum on a MIDI note-on event.
    ///
    /// - `note`    : MIDI note number 0-127.
    /// - `velocity`: 0.0–1.0.
    /// - `tuning`  : Semitone offset (±24).
    /// - `decay`   : T60 time in seconds.
    /// - `damp`    : Frequency-dependent damping [0, 1].
    /// - `strike`  : Hardness [0, 1] — hard = flat spectrum, soft = low modes only.
    pub fn trigger(
        &mut self,
        note: u8,
        velocity: f32,
        tuning: f32,
        decay: f32,
        damp: f32,
        strike: f32,
    ) {
        let fundamental = 440.0 * 2.0_f32.powf((note as f32 - 69.0 + tuning) / 12.0);
        let velocity = velocity.clamp(0.0, 1.0);

        // Per-sample base decay: λ = (1/1000)^(1/(Fs·T60))  [eq. 3.19]
        let base_decay = (1.0_f32 / 1000.0_f32).powf(1.0 / (self.sample_rate * decay.max(0.001)));

        for (i, mode) in self.modes.iter_mut().enumerate() {
            let ratio = MODE_RATIOS[i];
            let freq = fundamental * ratio;

            if freq >= self.sample_rate * 0.49 {
                mode.amplitude = 0.0;
                continue;
            }

            // Strike: exponential amplitude rolloff — hard=flat, soft=only low modes.
            let falloff = (1.0 - strike) * 3.0;
            let amp = BASE_AMPLITUDES[i] * (-falloff * i as f32).exp() * velocity;

            // Frequency-dependent damping: higher modes decay faster when damp > 0.
            let mode_decay = base_decay.powf(1.0 + damp * ratio);

            mode.setup(freq, amp, mode_decay, self.sample_rate);
        }

        self.lp_state = 0.0;
        self.active = true;
    }

    pub fn is_active(&self) -> bool {
        self.active
    }

    /// Generate one output sample (mono), applying L/Cut and attenuation.
    ///
    /// - `lcut`: Low-pass cutoff [0, 1] → exponential map 20 Hz–20 kHz.
    /// - `atten`: Output attenuation [0, 1].
    pub fn next_sample_mono(&mut self, lcut: f32, atten: f32) -> f32 {
        let fc = (20.0_f32 * 1000.0_f32.powf(lcut)).min(self.sample_rate * 0.499);
        let lp_coeff = (1.0 - (-TAU * fc / self.sample_rate).exp()).clamp(0.0, 1.0);
        let atten_gain = 1.0 - atten * 0.95;

        let mut out = 0.0_f32;
        for mode in self.modes.iter_mut() {
            out += mode.next_sample();
        }

        out *= atten_gain;
        self.lp_state += lp_coeff * (out - self.lp_state);

        self.active = self.modes.iter().any(|m| m.amplitude.abs() > 1e-6);
        self.lp_state
    }
}
