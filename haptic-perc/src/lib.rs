// Haptic Perc — Physical modelling percussion synthesizer CLAP plugin.
//
// DSP: Modal synthesis of a circular membrane using 8 damped sinusoidal
// resonators at the Bessel-function-zero frequency ratios (Chalmers 2005).
//
// Parameters (matching the Haptic Perc UI in reference.png):
//   TUNING   — Semitone offset from the MIDI note pitch (±24 st)
//   DECAY    — T60 decay time in seconds (0.05 – 4.0 s)
//   DAMP.    — Frequency-dependent damping: higher modes decay faster
//   STRIKE   — Strike hardness: hard = flat spectrum, soft = low modes only
//   ATTEN.   — Output attenuation of the resonator bank
//   L/CUT    — Low-pass filter cutoff (0 = 20 Hz, 1 = 20 kHz)
//   MIC GAIN — Microphone / pre-output gain (0 – 2×)
//   OUT GAIN — Master output level (0 – 1)

use clack_extensions::{audio_ports::*, note_ports::*, params::*};
use clack_plugin::events::spaces::CoreEventSpace;
use clack_plugin::prelude::*;

use crate::dsp::DrumVoice;
use crate::params::PercParams;

mod dsp;
mod params;

// ── Plugin entry point ────────────────────────────────────────────────────────

pub struct HapticPercPlugin;

impl Plugin for HapticPercPlugin {
    type AudioProcessor<'a> = HapticPercAudioProcessor<'a>;
    type Shared<'a> = HapticPercShared;
    type MainThread<'a> = HapticPercMainThread<'a>;

    fn declare_extensions(
        builder: &mut PluginExtensions<Self>,
        _shared: Option<&HapticPercShared>,
    ) {
        builder
            .register::<PluginAudioPorts>()
            .register::<PluginNotePorts>()
            .register::<PluginParams>();
    }
}

impl DefaultPluginFactory for HapticPercPlugin {
    fn get_descriptor() -> PluginDescriptor {
        use clack_plugin::plugin::features::*;
        PluginDescriptor::new("com.sampleson.haptic-perc", "Haptic Perc")
            .with_features([SYNTHESIZER, INSTRUMENT, STEREO])
    }

    fn new_shared(_host: HostSharedHandle) -> Result<HapticPercShared, PluginError> {
        Ok(HapticPercShared {
            params: PercParams::new(),
        })
    }

    fn new_main_thread<'a>(
        _host: HostMainThreadHandle<'a>,
        shared: &'a HapticPercShared,
    ) -> Result<HapticPercMainThread<'a>, PluginError> {
        Ok(HapticPercMainThread { shared })
    }
}

// ── Shared data (main thread + audio thread) ──────────────────────────────────

pub struct HapticPercShared {
    params: PercParams,
}

impl PluginShared<'_> for HapticPercShared {}

// ── Main thread ───────────────────────────────────────────────────────────────

pub struct HapticPercMainThread<'a> {
    shared: &'a HapticPercShared,
}

impl<'a> PluginMainThread<'a, HapticPercShared> for HapticPercMainThread<'a> {}

// Audio port: stereo output only (no audio input — instrument synthesizer).
impl PluginAudioPortsImpl for HapticPercMainThread<'_> {
    fn count(&mut self, is_input: bool) -> u32 {
        if is_input { 0 } else { 1 }
    }

    fn get(&mut self, index: u32, is_input: bool, writer: &mut AudioPortInfoWriter) {
        if !is_input && index == 0 {
            writer.set(&AudioPortInfo {
                id: ClapId::new(1),
                name: b"main",
                channel_count: 2,
                flags: AudioPortFlags::IS_MAIN,
                port_type: Some(AudioPortType::STEREO),
                in_place_pair: None,
            });
        }
    }
}

// Note port: MIDI / CLAP note input.
impl PluginNotePortsImpl for HapticPercMainThread<'_> {
    fn count(&mut self, is_input: bool) -> u32 {
        if is_input { 1 } else { 0 }
    }

    fn get(&mut self, index: u32, is_input: bool, writer: &mut NotePortInfoWriter) {
        if is_input && index == 0 {
            writer.set(&NotePortInfo {
                id: ClapId::new(1),
                name: b"main",
                preferred_dialect: Some(NoteDialect::Clap),
                supported_dialects: NoteDialects::CLAP,
            });
        }
    }
}

// ── Audio processor (audio thread) ────────────────────────────────────────────

pub struct HapticPercAudioProcessor<'a> {
    shared: &'a HapticPercShared,
    voice: DrumVoice,
}

impl<'a> PluginAudioProcessor<'a, HapticPercShared, HapticPercMainThread<'a>>
    for HapticPercAudioProcessor<'a>
{
    fn activate(
        _host: HostAudioProcessorHandle<'a>,
        _main_thread: &mut HapticPercMainThread,
        shared: &'a HapticPercShared,
        audio_config: PluginAudioConfiguration,
    ) -> Result<Self, PluginError> {
        Ok(Self {
            shared,
            voice: DrumVoice::new(audio_config.sample_rate as f32),
        })
    }

    fn process(
        &mut self,
        _process: Process,
        mut audio: Audio,
        events: Events,
    ) -> Result<ProcessStatus, PluginError> {
        let mut output_port = audio
            .output_port(0)
            .ok_or(PluginError::Message("No output port"))?;

        let mut output_channels = output_port
            .channels()?
            .into_f32()
            .ok_or(PluginError::Message("Expected f32 output"))?;

        // Get the mutable slice for channel 0 (mono processing).
        let output_buffer = output_channels
            .channel_mut(0)
            .ok_or(PluginError::Message("Expected at least one channel"))?;

        // Zero the output buffer; generate_samples() will add into it.
        output_buffer.fill(0.0);

        // Process sample-accurate event batches.
        for event_batch in events.input.batch() {
            for event in event_batch.events() {
                self.handle_event(event);
            }

            if self.voice.is_active() {
                let lcut = self.shared.params.get_lcut();
                let atten = self.shared.params.get_atten();
                let batch_buf = &mut output_buffer[event_batch.sample_bounds()];
                self.voice.generate_samples(batch_buf, lcut, atten);
            }
        }

        // Apply MIC GAIN × OUT GAIN to channel 0.
        let gain = self.shared.params.get_mic_gain() * self.shared.params.get_out_gain();
        for s in output_buffer.iter_mut() {
            *s *= gain;
        }

        // Copy mono channel 0 to all remaining channels (stereo output).
        if output_channels.channel_count() > 1 {
            let (first_channel, other_channels) = output_channels.split_at_mut(1);
            let first_channel = first_channel.channel(0).unwrap();
            for other_channel in other_channels {
                other_channel.copy_from_slice(first_channel);
            }
        }

        if self.voice.is_active() {
            Ok(ProcessStatus::Continue)
        } else {
            Ok(ProcessStatus::Sleep)
        }
    }
}

impl HapticPercAudioProcessor<'_> {
    fn handle_event(&mut self, event: &UnknownEvent) {
        match event.as_core_event() {
            Some(CoreEventSpace::NoteOn(e)) => {
                // e.key() returns Match<u16>; for a NoteOn it is always Specific.
                let key = e.key().into_specific().unwrap_or(60) as u8;
                let p = &self.shared.params;
                self.voice.trigger(
                    key,
                    e.velocity(),
                    p.get_tuning(),
                    p.get_decay(),
                    p.get_damp(),
                    p.get_strike(),
                );
            }
            Some(CoreEventSpace::NoteOff(_)) => {
                // Percussion does not sustain; NoteOff is ignored.
            }
            Some(CoreEventSpace::ParamValue(e)) => {
                self.shared.params.handle_param_value(e);
            }
            _ => {}
        }
    }
}

// ── CLAP entry point ──────────────────────────────────────────────────────────

clack_export_entry!(SinglePluginEntry<HapticPercPlugin>);
