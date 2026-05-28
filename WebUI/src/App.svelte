<script lang="ts">
  import { onDestroy } from 'svelte';
  import { params, setParameterValue, inputLevel, outputLevel } from './state/store';
  import { bridge } from './bridge/bridge';
  import type { ParameterId } from './types/parameters';
  import ParamSlider from './components/ParamSlider.svelte';
  import Camera from './components/Camera.svelte';
  import ObjectDetector from './components/ObjectDetector.svelte';
  import { detections } from './stores/detections';
  import { PRESETS } from './presets';

  let camActive = false;
  let camError  = '';

  // ── Detection stabilization + preset application ──────────────
  const STABLE_FRAMES = 10; // ~0.66 s at 15 fps detection rate
  let stableLabel: string | null = null;
  let stableCount = 0;
  let activePreset: string | null = null;

  function applyPreset(label: string) {
    const preset = PRESETS[label];
    if (!preset) return;
    for (const [id, value] of Object.entries(preset) as [ParameterId, number][]) {
      send(id, value);
    }
  }

  $: {
    const top = $detections[0] ?? null;
    const label = top?.label ?? null;
    if (label === stableLabel) {
      stableCount++;
      if (stableCount >= STABLE_FRAMES && label !== activePreset && label !== null) {
        activePreset = label;
        applyPreset(label);
      }
    } else {
      stableLabel = label;
      stableCount = 1;
    }
  }

  const { tuning, decay, damp, strike, atten, lcut, mic_gain, out_gain, threshold } = params;

  // Denormalize for display (matches JUCE NormalisableRange definitions)
  function denorm(id: ParameterId, n: number): string {
    switch (id) {
      case 'tuning':    return (-24 + n * 48).toFixed(1);
      case 'decay':     return (0.05 + Math.pow(n, 0.5) * 3.95).toFixed(2);
      case 'damp':      return (n * 100).toFixed(0);
      case 'strike':    return (n * 100).toFixed(0);
      case 'atten':     return (n * 100).toFixed(0);
      case 'lcut':      return Math.round(20 * Math.pow(1000, n)).toString();
      case 'mic_gain':  return (n * 2).toFixed(2);
      case 'out_gain':  return (-36 + Math.pow(n, 2.5) * 36).toFixed(1);
      case 'threshold': return (-60 + Math.pow(n, 2.5) * 60).toFixed(1);
    }
  }

  function send(id: ParameterId, v: number) {
    if (!isFinite(v) || isNaN(v)) return;
    setParameterValue(id, v);
    bridge.sendParameterChange(id, v);
  }

  // ── Clock ──────────────────────────────────────────────
  let clockStr = '--:--:--';
  const clockId = setInterval(() => {
    const t = new Date();
    clockStr = [t.getHours(), t.getMinutes(), t.getSeconds()]
      .map(n => String(n).padStart(2, '0')).join(':');
  }, 1000);

  // ── Spectrum bars (real — C++ FFT at ~43 Hz, ballistic smoothing in rAF) ──
  let spec: number[] = new Array(40).fill(0);
  let rawSpec: number[] = new Array(40).fill(0);
  // Registered once — C++ calls window.setSpectrum([...40 bins...]) at 30 Hz
  (window as any).setSpectrum = (data: number[]) => { rawSpec = data; };

  // ── Level meters (real — driven by C++ at 30 Hz, ballistic smoothing in rAF) ──
  let inLvl = 0, outLvl = 0, inPeak = 0, outPeak = 0;
  let lastT = performance.now();
  let meterRaf: number;

  // Raw peaks from C++ — written by store subscriptions, consumed by rAF
  let rawIn = 0, rawOut = 0;
  const unsubIn  = inputLevel.subscribe(v  => { rawIn  = v; });
  const unsubOut = outputLevel.subscribe(v => { rawOut = v; });

  function meterLoop(now: number) {
    const dt = (now - lastT) / 1000; lastT = now;
    // Attack fast (10 ms τ), release slow (300 ms τ)
    const aIn  = rawIn  > inLvl  ? Math.min(1, dt * 100) : Math.min(1, dt * 3.33);
    const aOut = rawOut > outLvl ? Math.min(1, dt * 100) : Math.min(1, dt * 3.33);
    inLvl   += (rawIn  - inLvl)  * aIn;
    outLvl  += (rawOut - outLvl) * aOut;
    // Peak hold: instant grab, slow fall (1.2 dB/s)
    inPeak  = Math.max(inPeak  * Math.pow(0.5, dt * 1.2), inLvl);
    outPeak = Math.max(outPeak * Math.pow(0.5, dt * 1.2), outLvl);
    // Spectrum ballistics: fast attack, slow release
    spec = spec.map((v, i) => {
      const t = rawSpec[i] ?? 0;
      return t > v ? v + (t - v) * 0.5 : v + (t - v) * 0.08;
    });
    meterRaf = requestAnimationFrame(meterLoop);
  }
  meterRaf = requestAnimationFrame(meterLoop);

  onDestroy(() => {
    clearInterval(clockId);
    cancelAnimationFrame(meterRaf);
    unsubIn();
    unsubOut();
  });

  // Build meter segment class arrays
  const SEGS = 28;
  function buildSegs(val: number, peak: number): string[] {
    const lit    = Math.round(val * SEGS);
    const pkIdx  = Math.round(peak * SEGS);
    return Array.from({ length: SEGS }, (_, i) => {
      const isLit  = i < lit;
      const isHot  = i / SEGS > 0.85;
      const isPeak = i === pkIdx - 1 && pkIdx > 0;
      if (isLit) return isHot ? 'hot' : 'lit';
      if (isPeak && !isLit) return 'lit';
      return '';
    });
  }

  $: inSegs  = buildSegs(inLvl,  inPeak);
  $: outSegs = buildSegs(outLvl, outPeak);
  $: inDb    = inLvl  > 0.001 ? (20 * Math.log10(inLvl)).toFixed(1)  : '-∞';
  $: outDb   = outLvl > 0.001 ? (20 * Math.log10(outLvl)).toFixed(1) : '-∞';
</script>

<!-- Headless MediaPipe object detector — subscribes to cameraStream, writes to detections store -->
<ObjectDetector />

<div class="app" data-palette="peach" data-scanlines="true" data-bloom="true">

  <!-- SVG bloom filter -->
  <svg width="0" height="0" style="position:absolute" aria-hidden="true">
    <defs>
      <filter id="crtBloom" x="-10%" y="-10%" width="120%" height="120%"
              color-interpolation-filters="sRGB">
        <feGaussianBlur in="SourceGraphic" stdDeviation="2.2" result="blur1"/>
        <feGaussianBlur in="SourceGraphic" stdDeviation="6"   result="blur2"/>
        <feColorMatrix  in="blur2" type="matrix"
          values="1 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 1.35 0" result="halo"/>
        <feMerge>
          <feMergeNode in="halo"/>
          <feMergeNode in="blur1"/>
          <feMergeNode in="SourceGraphic"/>
        </feMerge>
      </filter>
    </defs>
  </svg>

  <!-- ── Top bar ────────────────────────────────────────── -->
  <div class="topbar">
    <div class="brand">
      <div class="mark"><i></i><i></i><i></i><i></i><i></i><i></i><i></i><i></i><i></i></div>
      <div class="col">
        <span class="name display">perk<b>ung</b>·fu</span>
        <span class="sub">phys.model · perc.synth · v1.04</span>
      </div>
    </div>

    <div class="session">
      <span class="dot"></span>
      <span class="lbl">REDS Explorer Mk.2</span>
      <span class="sep">·</span>
      <span class="sid">MAGI//PerKung-fu.SYS</span>
    </div>

    <div class="clock">
      <span>BPM <b class="num">96</b></span>
      <span>SR <b class="num">48k</b></span>
      <span>↑ <b class="num">{clockStr}</b></span>
    </div>
  </div>

  <!-- ── Workspace ──────────────────────────────────────── -->
  <div class="workspace">

    <!-- LEFT: Resonator + Exciter -->
    <div class="params">
      <div class="panel pgroup">
        <div class="panel-hd">
          <span class="title">◢ Resonator</span>
          <span class="meta">String · Karplus-Strong</span>
        </div>
        <div class="pgroup-body">
          <ParamSlider name="Tuning" value={$tuning}
            display={denorm('tuning', $tuning)} unit="st"
            on:change={e => send('tuning', e.detail)} />
          <ParamSlider name="Decay" value={$decay}
            display={denorm('decay', $decay)} unit="s"
            on:change={e => send('decay', e.detail)} />
          <ParamSlider name="Damp" value={$damp}
            display={denorm('damp', $damp)} unit="%"
            on:change={e => send('damp', e.detail)} />
        </div>
      </div>

      <div class="panel pgroup">
        <div class="panel-hd">
          <span class="title">◢ Exciter</span>
          <span class="meta">Strike → Body</span>
        </div>
        <div class="pgroup-body">
          <ParamSlider name="Strike" value={$strike}
            display={denorm('strike', $strike)} unit="%"
            on:change={e => send('strike', e.detail)} />
          <ParamSlider name="Attenuation" value={$atten}
            display={denorm('atten', $atten)} unit="%"
            on:change={e => send('atten', e.detail)} />
          <ParamSlider name="L/Cut" value={$lcut}
            display={denorm('lcut', $lcut)} unit="Hz"
            on:change={e => send('lcut', e.detail)} />
        </div>
      </div>
    </div>

    <!-- CENTER: Camera + Spectrum -->
    <div class="center">
      <!-- Camera panel -->
      <div class="panel cam-outer">
        <div class="panel-hd">
          <span class="title">◢ Optic · Object Recognition</span>
          <span class="meta">CAM_01 · CASPER-3 model · {camActive ? 'LIVE' : 'OFFLINE'}</span>
        </div>
        <div class="cam-wrap">
          <div class="cam">
            <!-- Camera feed — auto-starts via getUserMedia -->
            <Camera bind:active={camActive} bind:error={camError} />

            <!-- HUD overlay -->
            <div class="corner tl"></div>
            <div class="corner tr"></div>
            <div class="corner bl"></div>
            <div class="corner br"></div>
            <div class="reticle"></div>
            <div class="scanline-sweep"></div>

            <div class="hud-top">
              <span>◉ REC · CAM_01</span>
              <span>F/2.4 · ISO_400</span>
              <span>{camActive ? 'LIVE' : camError ? 'ERR' : 'INIT'}</span>
            </div>
            <div class="hud-bot">
              <span>EXP 1/60</span>
              <span>OBJ_DETECT v3.14</span>
              <span>{String($detections.length).padStart(2, '0')} TGT</span>
            </div>
          </div>
        </div>
      </div>

      <!-- Spectrum strip -->
      <div class="panel spec-panel">
        <div class="panel-hd">
          <span class="title">◢ Spectrum · Post-FX</span>
          <span class="meta">FFT 1024 · log</span>
        </div>
        <div class="spec">
          {#each spec as v}
            <i style="height:{Math.max(2, v * 100)}%"></i>
          {/each}
        </div>
      </div>
    </div>

    <!-- RIGHT: I/O + Detections + Stats -->
    <div class="right">
      <div class="panel pgroup">
        <div class="panel-hd">
          <span class="title">◢ I/O · Gating</span>
          <span class="meta">Mic → Out</span>
        </div>
        <div class="pgroup-body">
          <ParamSlider name="Mic Gain" value={$mic_gain}
            display={denorm('mic_gain', $mic_gain)} unit="x"
            on:change={e => send('mic_gain', e.detail)} />
          <ParamSlider name="Out Gain" value={$out_gain}
            display={denorm('out_gain', $out_gain)} unit="dB"
            on:change={e => send('out_gain', e.detail)} />
          <ParamSlider name="Threshold" value={$threshold}
            display={denorm('threshold', $threshold)} unit="dB"
            on:change={e => send('threshold', e.detail)} />
        </div>
      </div>

      <!-- Detected objects (placeholder until MediaPipe is implemented) -->
      <div class="panel" style="flex:1;display:flex;flex-direction:column;">
        <div class="panel-hd">
          <span class="title">◢ Detected Objects</span>
          <span class="meta">MediaPipe · COCO</span>
        </div>
        <div class="det-list">
          {#if $detections.length === 0}
            <div class="det-empty">no targets in frame</div>
          {:else}
            {#each $detections as d}
              <div class="det-row" class:det-active={d.label === activePreset}>
                <span class="det-label">{d.label.toUpperCase()}</span>
                <span class="det-conf">{(d.score * 100).toFixed(0)}%</span>
                {#if d.label === activePreset}
                  <span class="det-badge">PRESET</span>
                {/if}
              </div>
            {/each}
          {/if}
        </div>
      </div>

      <!-- System stats -->
      <div class="panel stats">
        <div class="stat">
          <div class="k">Voices</div>
          <div class="v num acc">01 / 32</div>
        </div>
        <div class="stat">
          <div class="k">CPU</div>
          <div class="v num">--.-%</div>
        </div>
        <div class="stat">
          <div class="k">Latency</div>
          <div class="v num">-- ms</div>
        </div>
        <div class="stat">
          <div class="k">Buffer</div>
          <div class="v num">256</div>
        </div>
      </div>
    </div>

  </div><!-- /workspace -->

  <!-- ── Bottom rail ────────────────────────────────────── -->
  <div class="botbar">

    <!-- Mic In meter -->
    <div class="meter">
      <div class="mhd">
        <span>Mic In</span>
        <b>{inDb} dB</b>
      </div>
      <div class="meter-bar">
        {#each inSegs as cls}
          <i class={cls}></i>
        {/each}
      </div>
    </div>

    <!-- Status pills -->
    <div class="botcenter">
      <div class="pill"><span class="dot"></span> <span>STATUS</span> <b>NOMINAL</b></div>
      <div class="pill"><span>ENG</span> <b>BALTHASAR·2</b></div>
      <div class="pill"><span>PRESET</span> <b>DEFAULT</b></div>
      <div class="pill"><span>UPTIME</span> <b class="num">--:--:--</b></div>
    </div>

    <!-- Output meter -->
    <div class="meter">
      <div class="mhd">
        <span>Output</span>
        <b>{outDb} dB</b>
      </div>
      <div class="meter-bar">
        {#each outSegs as cls}
          <i class={cls}></i>
        {/each}
      </div>
    </div>

  </div>

</div>
