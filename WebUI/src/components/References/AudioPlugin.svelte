<script>
  import Knob from './Knob.svelte';
  import Slider from './Slider.svelte';
  import Toggle from './Toggle.svelte';
  import LedMatrix from './LedMatrix.svelte';
  import Selector from './Selector.svelte';
  import XYPad from './XYPad.svelte';

  let power = true;
  let bypass = false;
  let mono = false;
</script>

<div class="scene">
  <div class="plugin">

    <!-- Header -->
    <div class="header">
      <div class="brand">
        <span class="brand-name">ÆTHER</span>
        <span class="brand-model">— DSP·01</span>
      </div>
      <div class="header-controls">
        <Toggle label="PWR" bind:active={power} />
        <Toggle label="BYPASS" bind:active={bypass} />
        <Toggle label="MONO" bind:active={mono} />
      </div>
    </div>

    <div class="divider"></div>

    <!-- Main body -->
    <div class="body">

      <!-- Left: LED Matrix -->
      <div class="section">
        <LedMatrix cols={5} rows={9} />
      </div>

      <div class="vsep"></div>

      <!-- Center: Knobs cluster -->
      <div class="section center">
        <div class="knobs-grid">
          <Knob label="GAIN"   value={0.7}  min={-24} max={24}  unit=" dB" />
          <Knob label="DRIVE"  value={0.35} min={0}   max={100} unit="%" />
          <Knob label="TONE"   value={0.5}  min={0}   max={100} unit="%" />
          <Knob label="MIX"    value={0.6}  min={0}   max={100} unit="%" />
          <Knob label="SPACE"  value={0.45} min={0}   max={100} unit="%" />
          <Knob label="DEPTH"  value={0.8}  min={0}   max={100} unit="%" />
        </div>

        <div class="xypad-row">
          <XYPad labelX="CUTOFF" labelY="RESO" x={0.4} y={0.6} />

          <div class="sliders-row">
            <Slider label="ATK"  value={0.3}  min={0}  max={500} unit="ms" />
            <Slider label="DEC"  value={0.5}  min={0}  max={500} unit="ms" />
            <Slider label="SUS"  value={0.7}  min={0}  max={100} unit="%" />
            <Slider label="REL"  value={0.4}  min={0}  max={2000} unit="ms" />
          </div>
        </div>
      </div>

      <div class="vsep"></div>

      <!-- Right: Selectors -->
      <div class="section right">
        <Selector
          label="ALGORITHM"
          options={['REVERB', 'DELAY', 'CHORUS', 'FLANGER']}
          selected={0}
        />
        <div style="height: 16px"></div>
        <Selector
          label="CHARACTER"
          options={['WARM', 'BRIGHT', 'NEUTRAL']}
          selected={1}
        />
        <div style="height: 16px"></div>
        <Selector
          label="RATE"
          options={['44.1k', '48k', '96k']}
          selected={1}
        />
      </div>

    </div>

    <!-- Footer -->
    <div class="divider"></div>
    <div class="footer">
      <span class="footer-txt">ÆTHER DSP·01 — Spectral Processor</span>
      <div class="status-dot" class:on={power}></div>
    </div>

  </div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=Cinzel:wght@300;400&family=Jost:wght@200;300;400&display=swap');

  .scene {
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    background:
      radial-gradient(ellipse at 30% 40%, rgba(180,160,240,0.12), transparent 60%),
      radial-gradient(ellipse at 70% 70%, rgba(200,160,255,0.08), transparent 55%),
      #0a080f;
    padding: 2rem;
    font-family: 'Jost', sans-serif;
  }

  .plugin {
    position: relative;
    background: linear-gradient(
      135deg,
      rgba(255,255,255,0.09) 0%,
      rgba(200,190,240,0.06) 50%,
      rgba(160,140,220,0.08) 100%
    );
    backdrop-filter: blur(24px) saturate(160%);
    -webkit-backdrop-filter: blur(24px) saturate(160%);
    border: 1px solid rgba(255,255,255,0.16);
    border-radius: 20px;
    box-shadow:
      0 24px 64px rgba(0,0,0,0.55),
      inset 0 1px 0 rgba(255,255,255,0.2),
      inset 0 -1px 0 rgba(255,255,255,0.05);
    padding: 24px 28px;
    max-width: 720px;
    width: 100%;
  }

  /* ── Header ── */
  .header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 16px;
  }

  .brand {
    display: flex;
    align-items: baseline;
    gap: 8px;
  }

  .brand-name {
    font-family: 'Cinzel', serif;
    font-size: 1.3rem;
    font-weight: 400;
    letter-spacing: 0.3em;
    color: rgba(230, 220, 255, 0.9);
    text-shadow: 0 0 20px rgba(200,180,255,0.3);
  }

  .brand-model {
    font-size: 0.65rem;
    font-weight: 200;
    letter-spacing: 0.25em;
    color: rgba(180,170,210,0.5);
  }

  .header-controls {
    display: flex;
    gap: 18px;
    align-items: flex-end;
  }

  /* ── Divider ── */
  .divider {
    height: 1px;
    background: linear-gradient(to right, transparent, rgba(255,255,255,0.1), transparent);
    margin: 0 -4px 20px;
  }

  /* ── Body ── */
  .body {
    display: flex;
    gap: 0;
    align-items: flex-start;
  }

  .section {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 0 16px;
  }

  .section.center {
    flex: 1;
    align-items: stretch;
  }

  .section.right {
    align-items: flex-start;
    padding-right: 0;
  }

  .vsep {
    width: 1px;
    align-self: stretch;
    background: linear-gradient(to bottom, transparent, rgba(255,255,255,0.1), transparent);
    margin: 0 4px;
  }

  /* ── Knobs grid ── */
  .knobs-grid {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 16px 24px;
    justify-items: center;
    margin-bottom: 20px;
  }

  /* ── XY + Sliders row ── */
  .xypad-row {
    display: flex;
    gap: 20px;
    align-items: flex-end;
  }

  .sliders-row {
    display: flex;
    gap: 14px;
    align-items: flex-end;
  }

  /* ── Footer ── */
  .footer {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-top: 16px;
  }

  .footer-txt {
    font-size: 0.55rem;
    font-weight: 200;
    letter-spacing: 0.25em;
    text-transform: uppercase;
    color: rgba(180,170,210,0.35);
  }

  .status-dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: rgba(180,100,60,0.4);
    transition: background 0.3s, box-shadow 0.3s;
  }

  .status-dot.on {
    background: rgb(255, 110, 40);
    box-shadow: 0 0 8px rgba(255,90,30,0.8);
  }
</style>
