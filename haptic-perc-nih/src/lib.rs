// Haptic Perc — Physical modelling percussion synthesizer.
//
// Exports: CLAP, VST3, and (with --features standalone) a standalone application.
//
// DSP: 8-mode modal synthesis of a circular membrane using Bessel function zero
// frequency ratios. See dsp.rs and the Chalmers 2005 paper for details.

use nih_plug::prelude::*;
use nih_plug_egui::{create_egui_editor, egui, widgets, EguiState};
use std::sync::Arc;

mod dsp;
use dsp::DrumVoice;

// ── Plugin struct ─────────────────────────────────────────────────────────────

pub struct HapticPercNih {
    params: Arc<HapticPercParams>,
    voice: DrumVoice,
}

impl Default for HapticPercNih {
    fn default() -> Self {
        Self {
            params: Arc::new(HapticPercParams::default()),
            voice: DrumVoice::new(44100.0), // updated in initialize()
        }
    }
}

// ── Parameters ────────────────────────────────────────────────────────────────

#[derive(Params)]
struct HapticPercParams {
    /// Persisted editor window state (size, open/closed).
    #[persist = "editor-state"]
    editor_state: Arc<EguiState>,

    /// Semitone offset applied to the incoming MIDI note pitch.
    #[id = "tuning"]
    pub tuning: FloatParam,

    /// T60 decay time — how long the drum rings before reaching -60 dB.
    #[id = "decay"]
    pub decay: FloatParam,

    /// Frequency-dependent damping: higher modes decay faster as damp increases.
    #[id = "damp"]
    pub damp: FloatParam,

    /// Strike hardness: hard = flat spectrum across all modes,
    /// soft = energy concentrated in the lowest mode.
    #[id = "strike"]
    pub strike: FloatParam,

    /// Resonator attenuation — reduces the overall output level of the modal bank.
    #[id = "atten"]
    pub atten: FloatParam,

    /// Low-pass filter cutoff, mapped exponentially from 20 Hz (0) to 20 kHz (1).
    #[id = "lcut"]
    pub lcut: FloatParam,

    /// Microphone / pre-output gain multiplier.
    #[id = "mic_gain"]
    pub mic_gain: FloatParam,

    /// Master output level.
    #[id = "out_gain"]
    pub out_gain: FloatParam,
}

impl Default for HapticPercParams {
    fn default() -> Self {
        Self {
            editor_state: EguiState::from_size(380, 300),

            tuning: FloatParam::new(
                "Tuning",
                0.0,
                FloatRange::Linear { min: -24.0, max: 24.0 },
            )
            .with_unit(" st")
            .with_step_size(0.1),

            decay: FloatParam::new(
                "Decay",
                0.5,
                FloatRange::Skewed {
                    min: 0.05,
                    max: 4.0,
                    factor: FloatRange::skew_factor(-1.0),
                },
            )
            .with_unit(" s")
            .with_value_to_string(formatters::v2s_f32_rounded(2))
            .with_smoother(SmoothingStyle::None),

            damp: FloatParam::new(
                "Damp",
                0.3,
                FloatRange::Linear { min: 0.0, max: 1.0 },
            )
            .with_value_to_string(formatters::v2s_f32_percentage(1))
            .with_unit("%"),

            strike: FloatParam::new(
                "Strike",
                0.5,
                FloatRange::Linear { min: 0.0, max: 1.0 },
            )
            .with_value_to_string(formatters::v2s_f32_percentage(1))
            .with_unit("%"),

            atten: FloatParam::new(
                "Attenuation",
                0.0,
                FloatRange::Linear { min: 0.0, max: 1.0 },
            )
            .with_value_to_string(formatters::v2s_f32_percentage(1))
            .with_unit("%"),

            lcut: FloatParam::new(
                "L/Cut",
                1.0,
                FloatRange::Linear { min: 0.0, max: 1.0 },
            )
            .with_value_to_string(formatters::v2s_f32_percentage(1))
            .with_unit("%"),

            mic_gain: FloatParam::new(
                "Mic Gain",
                1.0,
                FloatRange::Linear { min: 0.0, max: 2.0 },
            )
            .with_value_to_string(formatters::v2s_f32_rounded(2))
            .with_unit("x"),

            out_gain: FloatParam::new(
                "Out Gain",
                util::db_to_gain(-1.5),
                FloatRange::Skewed {
                    min: util::db_to_gain(-36.0),
                    max: util::db_to_gain(0.0),
                    factor: FloatRange::gain_skew_factor(-36.0, 0.0),
                },
            )
            .with_smoother(SmoothingStyle::Linear(5.0))
            .with_unit(" dB")
            .with_value_to_string(formatters::v2s_f32_gain_to_db(1))
            .with_string_to_value(formatters::s2v_f32_gain_to_db()),
        }
    }
}

// ── Plugin trait ──────────────────────────────────────────────────────────────

impl Plugin for HapticPercNih {
    const NAME: &'static str = "Haptic Perc";
    const VENDOR: &'static str = "Sampleson";
    const URL: &'static str = "https://github.com/IAndy-10/PerKung-fu";
    const EMAIL: &'static str = "";
    const VERSION: &'static str = env!("CARGO_PKG_VERSION");

    // Stereo output only — instrument synthesizer, no audio input.
    const AUDIO_IO_LAYOUTS: &'static [AudioIOLayout] = &[AudioIOLayout {
        main_input_channels: None,
        main_output_channels: NonZeroU32::new(2),
        ..AudioIOLayout::const_default()
    }];

    const MIDI_INPUT: MidiConfig = MidiConfig::Basic;
    const SAMPLE_ACCURATE_AUTOMATION: bool = true;

    type SysExMessage = ();
    type BackgroundTask = ();

    fn params(&self) -> Arc<dyn Params> {
        self.params.clone()
    }

    fn editor(&mut self, _async_executor: AsyncExecutor<Self>) -> Option<Box<dyn Editor>> {
        let params = self.params.clone();
        create_egui_editor(
            self.params.editor_state.clone(),
            (),
            |_, _| {},
            move |ctx, setter, _state| {
                egui::CentralPanel::default().show(ctx, |ui| {
                    ui.heading("Haptic Perc");
                    ui.separator();
                    egui::Grid::new("params")
                        .num_columns(2)
                        .spacing([12.0, 6.0])
                        .striped(true)
                        .show(ui, |ui| {
                            ui.label("Tuning");
                            ui.add(widgets::ParamSlider::for_param(&params.tuning, setter));
                            ui.end_row();

                            ui.label("Decay");
                            ui.add(widgets::ParamSlider::for_param(&params.decay, setter));
                            ui.end_row();

                            ui.label("Damp");
                            ui.add(widgets::ParamSlider::for_param(&params.damp, setter));
                            ui.end_row();

                            ui.label("Strike");
                            ui.add(widgets::ParamSlider::for_param(&params.strike, setter));
                            ui.end_row();

                            ui.label("Attenuation");
                            ui.add(widgets::ParamSlider::for_param(&params.atten, setter));
                            ui.end_row();

                            ui.label("L/Cut");
                            ui.add(widgets::ParamSlider::for_param(&params.lcut, setter));
                            ui.end_row();

                            ui.label("Mic Gain");
                            ui.add(widgets::ParamSlider::for_param(&params.mic_gain, setter));
                            ui.end_row();

                            ui.label("Out Gain");
                            ui.add(widgets::ParamSlider::for_param(&params.out_gain, setter));
                            ui.end_row();
                        });
                });
            },
        )
    }

    fn initialize(
        &mut self,
        _audio_io_layout: &AudioIOLayout,
        buffer_config: &BufferConfig,
        _context: &mut impl InitContext<Self>,
    ) -> bool {
        self.voice = DrumVoice::new(buffer_config.sample_rate);
        true
    }

    fn reset(&mut self) {
        // DrumVoice decays naturally; nothing to reset explicitly.
    }

    fn process(
        &mut self,
        buffer: &mut Buffer,
        _aux: &mut AuxiliaryBuffers,
        context: &mut impl ProcessContext<Self>,
    ) -> ProcessStatus {
        let mut next_event = context.next_event();

        for (sample_id, channel_samples) in buffer.iter_samples().enumerate() {
            // Dispatch sample-accurate events at or before this sample.
            while let Some(event) = next_event {
                if event.timing() > sample_id as u32 {
                    break;
                }
                match event {
                    NoteEvent::NoteOn { note, velocity, .. } => {
                        self.voice.trigger(
                            note,
                            velocity,
                            self.params.tuning.value(),
                            self.params.decay.value(),
                            self.params.damp.value(),
                            self.params.strike.value(),
                        );
                    }
                    // Percussion doesn't sustain — NoteOff is intentionally ignored.
                    _ => {}
                }
                next_event = context.next_event();
            }

            // Generate one mono drum sample and write to all output channels.
            let sample = self.voice.next_sample_mono(
                self.params.lcut.value(),
                self.params.atten.value(),
            ) * self.params.mic_gain.value()
                * self.params.out_gain.smoothed.next();

            for s in channel_samples {
                *s = sample;
            }
        }

        // KeepAlive keeps the plugin running even when no notes are playing,
        // so the natural decay tail is always rendered.
        ProcessStatus::KeepAlive
    }
}

// ── CLAP ──────────────────────────────────────────────────────────────────────

impl ClapPlugin for HapticPercNih {
    const CLAP_ID: &'static str = "com.sampleson.haptic-perc";
    const CLAP_DESCRIPTION: Option<&'static str> =
        Some("Physical modelling percussion synthesizer");
    const CLAP_MANUAL_URL: Option<&'static str> = None;
    const CLAP_SUPPORT_URL: Option<&'static str> = None;
    const CLAP_FEATURES: &'static [ClapFeature] = &[
        ClapFeature::Instrument,
        ClapFeature::Synthesizer,
        ClapFeature::Stereo,
    ];
}

// ── VST3 ──────────────────────────────────────────────────────────────────────

impl Vst3Plugin for HapticPercNih {
    const VST3_CLASS_ID: [u8; 16] = *b"HapticPercPlugin";
    const VST3_SUBCATEGORIES: &'static [Vst3SubCategory] =
        &[Vst3SubCategory::Instrument, Vst3SubCategory::Synth];
}

// ── Export macros ─────────────────────────────────────────────────────────────

nih_export_clap!(HapticPercNih);
nih_export_vst3!(HapticPercNih);
