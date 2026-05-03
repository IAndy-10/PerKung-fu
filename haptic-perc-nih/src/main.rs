// Standalone entry point.
// Build with: cargo build --release --features standalone --bin haptic-perc

use haptic_perc_nih::HapticPercNih;

fn main() {
    nih_plug::nih_export_standalone::<HapticPercNih>();
}
