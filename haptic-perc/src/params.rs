// Parameter management for Haptic Perc.
//
// Parameters map directly to the controls visible in reference.png:
//   Left sliders:  L/CUT, MIC GATE (stub), MIC GAIN, DECAY
//   Right sliders: ATTEN., TUNING, DAMP., STRIKE
//   Bottom knobs:  OUT GAIN (AMBIENT and WIDTH are future work)

use crate::{HapticPercAudioProcessor, HapticPercMainThread};
use clack_extensions::params::*;
use clack_plugin::events::event_types::ParamValueEvent;
use clack_plugin::events::spaces::CoreEventSpace;
use clack_plugin::prelude::*;
use std::ffi::CStr;
use std::fmt::Write as _;
use std::sync::atomic::{AtomicU32, Ordering};

// ── Parameter IDs ─────────────────────────────────────────────────────────────

pub const PARAM_TUNING_ID: ClapId = ClapId::new(1);
pub const PARAM_DECAY_ID: ClapId = ClapId::new(2);
pub const PARAM_DAMP_ID: ClapId = ClapId::new(3);
pub const PARAM_STRIKE_ID: ClapId = ClapId::new(4);
pub const PARAM_ATTEN_ID: ClapId = ClapId::new(5);
pub const PARAM_LCUT_ID: ClapId = ClapId::new(6);
pub const PARAM_MIC_GAIN_ID: ClapId = ClapId::new(7);
pub const PARAM_OUT_GAIN_ID: ClapId = ClapId::new(8);

pub const PARAM_COUNT: u32 = 8;

// ── Default values ─────────────────────────────────────────────────────────────

const DEFAULT_TUNING: f32 = 0.0;   // semitones
const DEFAULT_DECAY: f32 = 0.5;    // seconds
const DEFAULT_DAMP: f32 = 0.3;
const DEFAULT_STRIKE: f32 = 0.5;
const DEFAULT_ATTEN: f32 = 0.0;
const DEFAULT_LCUT: f32 = 1.0;     // fully open (20 kHz)
const DEFAULT_MIC_GAIN: f32 = 1.0;
const DEFAULT_OUT_GAIN: f32 = 0.8;

// ── PercParams ─────────────────────────────────────────────────────────────────

/// All plugin parameters stored as atomics for lock-free sharing between
/// the main thread and the audio thread.
pub struct PercParams {
    tuning: AtomicF32,
    decay: AtomicF32,
    damp: AtomicF32,
    strike: AtomicF32,
    atten: AtomicF32,
    lcut: AtomicF32,
    mic_gain: AtomicF32,
    out_gain: AtomicF32,
}

impl PercParams {
    pub fn new() -> Self {
        Self {
            tuning: AtomicF32::new(DEFAULT_TUNING),
            decay: AtomicF32::new(DEFAULT_DECAY),
            damp: AtomicF32::new(DEFAULT_DAMP),
            strike: AtomicF32::new(DEFAULT_STRIKE),
            atten: AtomicF32::new(DEFAULT_ATTEN),
            lcut: AtomicF32::new(DEFAULT_LCUT),
            mic_gain: AtomicF32::new(DEFAULT_MIC_GAIN),
            out_gain: AtomicF32::new(DEFAULT_OUT_GAIN),
        }
    }

    // ── Getters ───────────────────────────────────────────────────────────────

    pub fn get_tuning(&self) -> f32 {
        self.tuning.load()
    }
    pub fn get_decay(&self) -> f32 {
        self.decay.load()
    }
    pub fn get_damp(&self) -> f32 {
        self.damp.load()
    }
    pub fn get_strike(&self) -> f32 {
        self.strike.load()
    }
    pub fn get_atten(&self) -> f32 {
        self.atten.load()
    }
    pub fn get_lcut(&self) -> f32 {
        self.lcut.load()
    }
    pub fn get_mic_gain(&self) -> f32 {
        self.mic_gain.load()
    }
    pub fn get_out_gain(&self) -> f32 {
        self.out_gain.load()
    }

    // ── Event handler ─────────────────────────────────────────────────────────

    /// Apply a single `ParamValueEvent` to the matching parameter.
    pub fn handle_param_value(&self, event: &ParamValueEvent) {
        let v = event.value() as f32;
        match event.param_id() {
            id if id == PARAM_TUNING_ID => self.tuning.store(v.clamp(-24.0, 24.0)),
            id if id == PARAM_DECAY_ID => self.decay.store(v.clamp(0.05, 4.0)),
            id if id == PARAM_DAMP_ID => self.damp.store(v.clamp(0.0, 1.0)),
            id if id == PARAM_STRIKE_ID => self.strike.store(v.clamp(0.0, 1.0)),
            id if id == PARAM_ATTEN_ID => self.atten.store(v.clamp(0.0, 1.0)),
            id if id == PARAM_LCUT_ID => self.lcut.store(v.clamp(0.0, 1.0)),
            id if id == PARAM_MIC_GAIN_ID => self.mic_gain.store(v.clamp(0.0, 2.0)),
            id if id == PARAM_OUT_GAIN_ID => self.out_gain.store(v.clamp(0.0, 1.0)),
            _ => {}
        }
    }
}

// ── Main-thread parameter interface ───────────────────────────────────────────

impl PluginMainThreadParams for HapticPercMainThread<'_> {
    fn count(&mut self) -> u32 {
        PARAM_COUNT
    }

    fn get_info(&mut self, param_index: u32, info: &mut ParamInfoWriter) {
        match param_index {
            0 => info.set(&ParamInfo {
                id: PARAM_TUNING_ID,
                flags: ParamInfoFlags::IS_AUTOMATABLE,
                cookie: Default::default(),
                name: b"Tuning",
                module: b"",
                min_value: -24.0,
                max_value: 24.0,
                default_value: DEFAULT_TUNING as f64,
            }),
            1 => info.set(&ParamInfo {
                id: PARAM_DECAY_ID,
                flags: ParamInfoFlags::IS_AUTOMATABLE,
                cookie: Default::default(),
                name: b"Decay",
                module: b"",
                min_value: 0.05,
                max_value: 4.0,
                default_value: DEFAULT_DECAY as f64,
            }),
            2 => info.set(&ParamInfo {
                id: PARAM_DAMP_ID,
                flags: ParamInfoFlags::IS_AUTOMATABLE,
                cookie: Default::default(),
                name: b"Damp",
                module: b"",
                min_value: 0.0,
                max_value: 1.0,
                default_value: DEFAULT_DAMP as f64,
            }),
            3 => info.set(&ParamInfo {
                id: PARAM_STRIKE_ID,
                flags: ParamInfoFlags::IS_AUTOMATABLE,
                cookie: Default::default(),
                name: b"Strike",
                module: b"",
                min_value: 0.0,
                max_value: 1.0,
                default_value: DEFAULT_STRIKE as f64,
            }),
            4 => info.set(&ParamInfo {
                id: PARAM_ATTEN_ID,
                flags: ParamInfoFlags::IS_AUTOMATABLE,
                cookie: Default::default(),
                name: b"Attenuation",
                module: b"",
                min_value: 0.0,
                max_value: 1.0,
                default_value: DEFAULT_ATTEN as f64,
            }),
            5 => info.set(&ParamInfo {
                id: PARAM_LCUT_ID,
                flags: ParamInfoFlags::IS_AUTOMATABLE,
                cookie: Default::default(),
                name: b"L/Cut",
                module: b"",
                min_value: 0.0,
                max_value: 1.0,
                default_value: DEFAULT_LCUT as f64,
            }),
            6 => info.set(&ParamInfo {
                id: PARAM_MIC_GAIN_ID,
                flags: ParamInfoFlags::IS_AUTOMATABLE,
                cookie: Default::default(),
                name: b"Mic Gain",
                module: b"",
                min_value: 0.0,
                max_value: 2.0,
                default_value: DEFAULT_MIC_GAIN as f64,
            }),
            7 => info.set(&ParamInfo {
                id: PARAM_OUT_GAIN_ID,
                flags: ParamInfoFlags::IS_AUTOMATABLE,
                cookie: Default::default(),
                name: b"Out Gain",
                module: b"",
                min_value: 0.0,
                max_value: 1.0,
                default_value: DEFAULT_OUT_GAIN as f64,
            }),
            _ => {}
        }
    }

    fn get_value(&mut self, param_id: ClapId) -> Option<f64> {
        if param_id == PARAM_TUNING_ID {
            Some(self.shared.params.get_tuning() as f64)
        } else if param_id == PARAM_DECAY_ID {
            Some(self.shared.params.get_decay() as f64)
        } else if param_id == PARAM_DAMP_ID {
            Some(self.shared.params.get_damp() as f64)
        } else if param_id == PARAM_STRIKE_ID {
            Some(self.shared.params.get_strike() as f64)
        } else if param_id == PARAM_ATTEN_ID {
            Some(self.shared.params.get_atten() as f64)
        } else if param_id == PARAM_LCUT_ID {
            Some(self.shared.params.get_lcut() as f64)
        } else if param_id == PARAM_MIC_GAIN_ID {
            Some(self.shared.params.get_mic_gain() as f64)
        } else if param_id == PARAM_OUT_GAIN_ID {
            Some(self.shared.params.get_out_gain() as f64)
        } else {
            None
        }
    }

    fn value_to_text(
        &mut self,
        param_id: ClapId,
        value: f64,
        writer: &mut ParamDisplayWriter,
    ) -> std::fmt::Result {
        if param_id == PARAM_TUNING_ID {
            write!(writer, "{value:+.1} st")
        } else if param_id == PARAM_DECAY_ID {
            write!(writer, "{value:.2} s")
        } else if param_id == PARAM_DAMP_ID || param_id == PARAM_STRIKE_ID || param_id == PARAM_ATTEN_ID || param_id == PARAM_LCUT_ID {
            write!(writer, "{:.0}%", value * 100.0)
        } else if param_id == PARAM_MIC_GAIN_ID {
            write!(writer, "{value:.2}x")
        } else if param_id == PARAM_OUT_GAIN_ID {
            if value > 0.0 {
                write!(writer, "{:.1} dB", 20.0 * value.log10())
            } else {
                write!(writer, "-inf dB")
            }
        } else {
            Err(std::fmt::Error)
        }
    }

    fn text_to_value(&mut self, param_id: ClapId, text: &CStr) -> Option<f64> {
        let s = text.to_str().ok()?;
        // Strip common suffixes and parse
        let stripped = s
            .trim()
            .trim_end_matches("st")
            .trim_end_matches('s')
            .trim_end_matches('%')
            .trim_end_matches('x')
            .trim_end_matches("dB")
            .trim();
        let v: f64 = stripped.parse().ok()?;
        if param_id == PARAM_TUNING_ID {
            Some(v.clamp(-24.0, 24.0))
        } else if param_id == PARAM_DECAY_ID {
            Some(v.clamp(0.05, 4.0))
        } else if param_id == PARAM_DAMP_ID
            || param_id == PARAM_STRIKE_ID
            || param_id == PARAM_ATTEN_ID
            || param_id == PARAM_LCUT_ID
        {
            // Value could be a percentage (0-100) or a ratio (0-1)
            if v > 1.0 {
                Some((v / 100.0).clamp(0.0, 1.0))
            } else {
                Some(v.clamp(0.0, 1.0))
            }
        } else if param_id == PARAM_MIC_GAIN_ID {
            Some(v.clamp(0.0, 2.0))
        } else if param_id == PARAM_OUT_GAIN_ID {
            Some(v.clamp(0.0, 1.0))
        } else {
            None
        }
    }

    fn flush(
        &mut self,
        input_parameter_changes: &InputEvents,
        _output_parameter_changes: &mut OutputEvents,
    ) {
        for event in input_parameter_changes {
            if let Some(CoreEventSpace::ParamValue(e)) = event.as_core_event() {
                self.shared.params.handle_param_value(e);
            }
        }
    }
}

// ── Audio-thread parameter interface ──────────────────────────────────────────

impl PluginAudioProcessorParams for HapticPercAudioProcessor<'_> {
    fn flush(
        &mut self,
        input_parameter_changes: &InputEvents,
        _output_parameter_changes: &mut OutputEvents,
    ) {
        for event in input_parameter_changes {
            self.handle_event(event);
        }
    }
}

// ── AtomicF32 helper ──────────────────────────────────────────────────────────

struct AtomicF32(AtomicU32);

impl AtomicF32 {
    fn new(value: f32) -> Self {
        Self(AtomicU32::new(value.to_bits()))
    }

    fn store(&self, value: f32) {
        self.0.store(value.to_bits(), Ordering::Relaxed);
    }

    fn load(&self) -> f32 {
        f32::from_bits(self.0.load(Ordering::Relaxed))
    }
}
