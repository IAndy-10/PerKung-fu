<script>
  import NeuKnob     from './NeuKnob.svelte';
  import NeuSlider   from './NeuSlider.svelte';
  import NeuButton   from './NeuButton.svelte';
  import NeuSelector from './NeuSelector.svelte';
  import NeuXYPad    from './NeuXYPad.svelte';
</script>

<div class="scene">
  <div class="panel">

    <!-- Header -->
    <div class="header">
      <div class="logo-area">
        <div class="logo-circle">
          <div class="logo-inner"></div>
        </div>
        <div>
          <div class="brand">FORMA</div>
          <div class="model">Analog · Series II</div>
        </div>
      </div>
      <div class="header-btns">
        <NeuButton label="PWR"    icon="◉" active={true} />
        <NeuButton label="BYPASS" icon="⇥" />
      </div>
    </div>

    <div class="rule"></div>

    <!-- Body -->
    <div class="body">

      <!-- Left col: sliders -->
      <div class="col sliders-col">
        <NeuSlider label="ATK"  value={0.25} min={0}  max={500} unit="ms" />
        <NeuSlider label="REL"  value={0.6}  min={0}  max={2000} unit="ms" />
        <NeuSlider label="SUS"  value={0.75} min={0}  max={100} unit="%" />
        <NeuSlider label="DEC"  value={0.4}  min={0}  max={500} unit="ms" />
      </div>

      <div class="vsep"></div>

      <!-- Center col -->
      <div class="col center-col">
        <!-- XY Pad (the orb) -->
        <NeuXYPad x={0.45} y={0.6} labelX="SPACE" labelY="DEPTH" />

        <div style="height: 16px"></div>

        <!-- Knobs row -->
        <div class="knobs-row">
          <NeuKnob label="DRIVE" value={0.55} min={0} max={100} unit="%" />
          <NeuKnob label="TONE"  value={0.4}  min={0} max={100} unit="%" />
          <NeuKnob label="MIX"   value={0.7}  min={0} max={100} unit="%" />
        </div>
      </div>

      <div class="vsep"></div>

      <!-- Right col -->
      <div class="col right-col">
        <NeuKnob label="GAIN"  value={0.65} min={-24} max={24} unit=" dB" />
        <NeuKnob label="WIDTH" value={0.5}  min={0}   max={100} unit="%" />

        <div style="height: 20px"></div>

        <NeuSelector
          label="MODE"
          options={['WARM', 'FLAT', 'AIR']}
          selected={0}
        />

        <div style="height: 14px"></div>

        <NeuSelector
          label="RATE"
          options={['44k', '48k', '96k']}
          selected={1}
        />

        <div style="height: 20px"></div>

        <div class="btn-row">
          <NeuButton label="MONO" icon="◎" />
          <NeuButton label="MUTE" icon="✕" />
        </div>
      </div>

    </div>

    <div class="rule"></div>

    <!-- Footer -->
    <div class="footer">
      <span>FORMA · Analog Series II · Spectral Processor</span>
      <div class="dots">
        <div class="dot active"></div>
        <div class="dot"></div>
        <div class="dot"></div>
      </div>
    </div>

  </div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:ital,wght@0,200;0,300;0,400;1,200&family=DM+Serif+Display:ital@0;1&display=swap');

  .scene {
    min-height: 100vh;
    background:
      radial-gradient(ellipse at 25% 35%, rgba(230,215,195,0.6), transparent 55%),
      radial-gradient(ellipse at 75% 70%, rgba(210,200,180,0.4), transparent 50%),
      #ede6da;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 2rem;
    font-family: 'DM Sans', sans-serif;
  }

  .panel {
    background: #ede6da;
    border-radius: 28px;
    box-shadow:
      14px 14px 34px rgba(155,135,105,0.38),
      -10px -10px 28px rgba(255,252,244,0.95);
    padding: 28px 32px;
    max-width: 740px;
    width: 100%;
  }

  /* ── Header ── */
  .header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 20px;
  }

  .logo-area {
    display: flex;
    align-items: center;
    gap: 14px;
  }

  .logo-circle {
    width: 44px;
    height: 44px;
    border-radius: 50%;
    background: #ede6da;
    box-shadow:
      5px 5px 12px rgba(150,130,100,0.4),
      -4px -4px 10px rgba(255,252,244,0.92);
    display: flex;
    align-items: center;
    justify-content: center;
  }

  .logo-inner {
    width: 24px;
    height: 24px;
    border-radius: 50%;
    background: radial-gradient(circle at 38% 32%, #f8f2e8, #d8cfc0);
    box-shadow:
      3px 3px 7px rgba(145,125,95,0.4),
      -2px -2px 5px rgba(255,250,240,0.85);
  }

  .brand {
    font-family: 'DM Serif Display', serif;
    font-size: 1.4rem;
    letter-spacing: 0.18em;
    color: rgba(110, 88, 60, 0.85);
  }

  .model {
    font-size: 0.6rem;
    font-weight: 200;
    letter-spacing: 0.2em;
    color: rgba(140, 115, 82, 0.55);
    margin-top: 1px;
  }

  .header-btns {
    display: flex;
    gap: 18px;
    align-items: flex-end;
  }

  /* ── Rule ── */
  .rule {
    height: 1px;
    background: linear-gradient(to right,
      transparent,
      rgba(150,130,100,0.2) 20%,
      rgba(150,130,100,0.2) 80%,
      transparent
    );
    margin: 0 -4px 24px;
  }

  /* ── Body ── */
  .body {
    display: flex;
    gap: 0;
    align-items: flex-start;
  }

  .col {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 0 18px;
  }

  .sliders-col {
    flex-direction: row;
    align-items: flex-end;
    gap: 14px;
    padding: 0 12px 0 0;
    align-self: center;
  }

  .center-col {
    flex: 1;
    align-items: center;
    padding: 0 20px;
  }

  .right-col {
    align-items: center;
    padding: 0 0 0 12px;
    gap: 14px;
  }

  .vsep {
    width: 1px;
    align-self: stretch;
    background: linear-gradient(
      to bottom,
      transparent,
      rgba(150,130,100,0.18) 30%,
      rgba(150,130,100,0.18) 70%,
      transparent
    );
    margin: 0 2px;
  }

  .knobs-row {
    display: flex;
    gap: 20px;
    justify-content: center;
  }

  .btn-row {
    display: flex;
    gap: 16px;
  }

  /* ── Footer ── */
  .footer {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-top: 20px;
    font-size: 0.52rem;
    font-weight: 200;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: rgba(140, 115, 82, 0.4);
  }

  .dots {
    display: flex;
    gap: 5px;
    align-items: center;
  }

  .dot {
    width: 5px;
    height: 5px;
    border-radius: 50%;
    background: #ede6da;
    box-shadow:
      2px 2px 4px rgba(150,130,100,0.4),
      -1px -1px 3px rgba(255,252,244,0.9);
  }

  .dot.active {
    background: #e2d9cb;
    box-shadow:
      inset 1px 1px 3px rgba(140,120,90,0.35),
      inset -1px -1px 2px rgba(255,250,240,0.8);
  }
</style>
