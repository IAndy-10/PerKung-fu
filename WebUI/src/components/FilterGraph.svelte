<script>
  import { onMount, onDestroy } from 'svelte';
  import { params, toNormalized } from '../state/store';
  import { bridge } from '../bridge/bridge';
  import NeuNumber from './NeuNumber.svelte';
  import NeuButton from './NeuButton.svelte';

  const { loCutEnabled, hiCutEnabled, loCutFreq, hiCutFreq, hiCutQ } = params;

  // ── Canvas constants ───────────────────────────────────────
  const CW = 300, CH = 110;
  const MX = 6,  MY = 8;
  const FMIN = 50, FMAX = 18000;
  const QMIN = 0.5, QMAX = 9.0;
  const DB_TOP = 6, DB_BOT = -48;
  // Lo Cut handle has no Q, so fix its Y at a readable position
  const LO_HANDLE_Y = Math.round(CH * 0.38);

  let canvas;

  // ── Drag state ─────────────────────────────────────────────
  let dragging       = false;
  let dragTarget     = null;  // 'lo' | 'hi'
  let dragStartCX, dragStartCY;
  let dragStartLoFreq, dragStartHiFreq, dragStartQ;
  let hoverTarget    = null;  // 'lo' | 'hi' | null

  // ── Display values (exact JUCE skew formula) ───────────────
  $: loCutFreqDisp = 50 + 17950 * Math.pow($loCutFreq, 1 / 0.3);
  $: hiCutFreqDisp = 50 + 17950 * Math.pow($hiCutFreq, 1 / 0.3);
  $: hiCutQDisp    = 0.5 + 8.5  * Math.pow($hiCutQ,   1 / 0.5);
  $: loCutOn       = $loCutEnabled > 0.5;
  $: hiCutOn       = $hiCutEnabled > 0.5;

  // ── Fake FFT background (3e) ───────────────────────────────
  const FFT_BINS = CW;
  // Initialise with a natural 1/f-like spectral shape (all negative = below 0 dB)
  const fftBins = new Float32Array(FFT_BINS);
  for (let i = 0; i < FFT_BINS; i++) {
    fftBins[i] = -0.15 - 0.65 * Math.pow(i / FFT_BINS, 0.55);
  }
  let animId = null;
  let lastFftTime = 0;

  function tickFft(time) {
    if (time - lastFftTime > 33) {   // ~30 fps
      lastFftTime = time;
      for (let i = 0; i < FFT_BINS; i++) {
        const natural = -0.15 - 0.65 * Math.pow(i / FFT_BINS, 0.55);
        fftBins[i] += (Math.random() - 0.5) * 0.05;
        fftBins[i]  = fftBins[i] * 0.92 + natural * 0.08;
        fftBins[i]  = Math.max(-1, Math.min(0, fftBins[i]));
      }
      draw();
    }
    animId = requestAnimationFrame(tickFft);
  }

  // 5b: onMount only starts the rAF loop — initial draw is handled by reactive block
  onMount(()   => { animId = requestAnimationFrame(tickFft); });
  onDestroy(() => { if (animId) cancelAnimationFrame(animId); });

  // ── Send helper ────────────────────────────────────────────
  function send(id, value) {
    params[id].set(value);
    bridge.sendParameterChange(id, value);
  }

  // ── Reactive canvas redraw (fires on param change AND on canvas bind) ─
  $: if (canvas) { loCutFreqDisp; hiCutFreqDisp; hiCutQDisp; loCutOn; hiCutOn; draw(); }

  // ── Coordinate conversions ─────────────────────────────────
  function freqToX(f) {
    return MX + (CW - 2 * MX) * Math.log10(f / FMIN) / Math.log10(FMAX / FMIN);
  }
  function xToFreq(x) {
    return FMIN * Math.pow(FMAX / FMIN, (x - MX) / (CW - 2 * MX));
  }
  function magToY(mag) {
    const db      = 20 * Math.log10(Math.max(mag, 1e-6));
    const clamped = Math.max(DB_BOT, Math.min(DB_TOP, db));
    return CH - MY - ((clamped - DB_BOT) / (DB_TOP - DB_BOT)) * (CH - 2 * MY);
  }
  function qToY(qv) {
    const norm = (Math.log(qv) - Math.log(QMIN)) / (Math.log(QMAX) - Math.log(QMIN));
    return MY + (1 - norm) * (CH - 2 * MY);
  }
  function yToQ(y) {
    const norm = Math.max(0, Math.min(1, 1 - (y - MY) / (CH - 2 * MY)));
    return Math.exp(norm * (Math.log(QMAX) - Math.log(QMIN)) + Math.log(QMIN));
  }

  // ── Filter transfer functions ──────────────────────────────
  function hpMag(f) {
    const r = f / loCutFreqDisp;
    return (r * r) / Math.sqrt(1 + r * r * r * r);
  }
  function lpMag(f) {
    const r = f / hiCutFreqDisp;
    return 1 / Math.sqrt((1 - r * r) * (1 - r * r) + (r / hiCutQDisp) * (r / hiCutQDisp));
  }

  // ── Draw ──────────────────────────────────────────────────
  function draw() {
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    // Background
    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, CW, CH);

    // Grid lines
    ctx.lineWidth = 0.5;
    ctx.strokeStyle = 'rgba(255,255,255,0.06)';
    for (const gf of [50, 100, 200, 500, 1000, 2000, 5000, 10000]) {
      const gx = freqToX(gf);
      ctx.beginPath(); ctx.moveTo(gx, 0); ctx.lineTo(gx, CH); ctx.stroke();
    }
    ctx.strokeStyle = 'rgba(255,255,255,0.04)';
    ctx.beginPath(); ctx.moveTo(0, CH / 2); ctx.lineTo(CW, CH / 2); ctx.stroke();

    // 3e: Frequency axis labels (100 Hz, 1k, 10k)
    ctx.font = '7px "DM Sans", sans-serif';
    ctx.fillStyle = 'rgba(255,255,255,0.15)';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'bottom';
    for (const [f, lbl] of [[100, '100'], [1000, '1k'], [10000, '10k']]) {
      ctx.fillText(lbl, freqToX(f), CH - 1);
    }

    // 3e: Fake FFT background spectrum
    ctx.beginPath();
    for (let px = 0; px <= CW; px++) {
      const v  = fftBins[Math.min(px, FFT_BINS - 1)]; // v ∈ [-1, 0]
      const py = CH - MY + v * (CH - 2 * MY);          // v<0 → py < CH-MY
      if (px === 0) ctx.moveTo(px, Math.max(0, py));
      else          ctx.lineTo(px, Math.max(0, py));
    }
    ctx.lineTo(CW, CH); ctx.lineTo(0, CH); ctx.closePath();
    ctx.fillStyle   = 'rgba(0, 200, 180, 0.05)';
    ctx.fill();
    ctx.beginPath();
    for (let px = 0; px <= CW; px++) {
      const v  = fftBins[Math.min(px, FFT_BINS - 1)];
      const py = CH - MY + v * (CH - 2 * MY);
      if (px === 0) ctx.moveTo(px, Math.max(0, py));
      else          ctx.lineTo(px, Math.max(0, py));
    }
    ctx.strokeStyle = 'rgba(0, 200, 180, 0.11)';
    ctx.lineWidth   = 0.6;
    ctx.stroke();

    // Magnitude curve
    const pts = [];
    for (let px = 0; px <= CW; px++) {
      const f = xToFreq(px);
      if (f < FMIN || f > FMAX) continue;
      let mag;
      if      (!loCutOn && !hiCutOn) mag = 1.0;
      else if ( loCutOn && !hiCutOn) mag = hpMag(f);
      else if (!loCutOn &&  hiCutOn) mag = lpMag(f);
      else                           mag = hpMag(f) * lpMag(f);
      pts.push([px, magToY(mag)]);
    }

    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.lineTo(CW, CH); ctx.lineTo(0, CH); ctx.closePath();
    ctx.fillStyle = 'rgba(0,200,180,0.09)';
    ctx.fill();

    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.strokeStyle = '#00c8b4';
    ctx.lineWidth   = 1.8; ctx.lineJoin = 'round'; ctx.stroke();

    // ── Lo Cut handle (teal) — X=freq only, Y fixed ─────────
    if (loCutOn) {
      const lhx  = freqToX(loCutFreqDisp);
      const lhy  = LO_HANDLE_Y;
      const isHov = hoverTarget === 'lo';
      const isDrg = dragging && dragTarget === 'lo';
      const r     = (isHov || isDrg) ? 7 : 5.5;

      // Dashed vertical line
      ctx.setLineDash([2, 3]);
      ctx.strokeStyle = 'rgba(0,200,180,0.12)';
      ctx.lineWidth   = 0.8;
      ctx.beginPath(); ctx.moveTo(lhx, MY); ctx.lineTo(lhx, CH - MY); ctx.stroke();
      ctx.setLineDash([]);

      // Glow on hover / drag
      if (isHov || isDrg) { ctx.shadowColor = 'rgba(0,200,180,0.55)'; ctx.shadowBlur = 10; }
      ctx.beginPath(); ctx.arc(lhx, lhy, r, 0, Math.PI * 2);
      ctx.fillStyle = isDrg ? '#20e8d4' : '#00c8b4';
      ctx.fill();
      ctx.shadowBlur = 0;
      ctx.beginPath(); ctx.arc(lhx, lhy, r, 0, Math.PI * 2);
      ctx.strokeStyle = 'rgba(150,240,220,0.4)'; ctx.lineWidth = 1; ctx.stroke();

      const loLabel = loCutFreqDisp >= 1000
        ? (loCutFreqDisp / 1000).toFixed(loCutFreqDisp % 1000 === 0 ? 0 : 1) + 'k'
        : Math.round(loCutFreqDisp).toString();
      ctx.font = '8px "DM Sans", sans-serif';
      ctx.fillStyle = 'rgba(0,200,180,0.5)';
      ctx.textAlign = 'center'; ctx.textBaseline = 'alphabetic';
      ctx.fillText(loLabel, lhx, CH - 1);
    }

    // ── Hi Cut handle (amber) — X=freq, Y=Q ─────────────────
    if (hiCutOn) {
      const hhx  = freqToX(hiCutFreqDisp);
      const hhy  = qToY(hiCutQDisp);
      const isHov = hoverTarget === 'hi';
      const isDrg = dragging && dragTarget === 'hi';
      const r     = (isHov || isDrg) ? 7 : 5.5;

      // Dashed vertical line (split around handle)
      ctx.setLineDash([2, 3]);
      ctx.strokeStyle = 'rgba(245,166,35,0.13)';
      ctx.lineWidth   = 0.8;
      ctx.beginPath(); ctx.moveTo(hhx, MY);        ctx.lineTo(hhx, hhy - r - 2); ctx.stroke();
      ctx.beginPath(); ctx.moveTo(hhx, hhy + r + 2); ctx.lineTo(hhx, CH - MY);  ctx.stroke();
      ctx.setLineDash([]);

      if (isHov || isDrg) { ctx.shadowColor = 'rgba(245,166,35,0.55)'; ctx.shadowBlur = 10; }
      ctx.beginPath(); ctx.arc(hhx, hhy, r, 0, Math.PI * 2);
      ctx.fillStyle = isDrg ? '#ffb940' : '#f5a623';
      ctx.fill();
      ctx.shadowBlur = 0;
      ctx.beginPath(); ctx.arc(hhx, hhy, r, 0, Math.PI * 2);
      ctx.strokeStyle = 'rgba(255,220,100,0.4)'; ctx.lineWidth = 1; ctx.stroke();

      const hiLabel = hiCutFreqDisp >= 1000
        ? (hiCutFreqDisp / 1000).toFixed(hiCutFreqDisp % 1000 === 0 ? 0 : 1) + 'k'
        : Math.round(hiCutFreqDisp).toString();
      ctx.font = '8px "DM Sans", sans-serif';
      ctx.fillStyle = 'rgba(245,166,35,0.5)';
      ctx.textAlign = 'center'; ctx.textBaseline = 'alphabetic';
      ctx.fillText(hiLabel, hhx, CH - 1);
    }
  }

  // ── Mouse: drag ────────────────────────────────────────────
  function onCanvasMouseDown(e) {
    const rect  = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const mx = (e.clientX - rect.left) * scaleX;
    const my = (e.clientY - rect.top)  * scaleY;

    let bestDist = 16;
    let target   = null;

    if (loCutOn) {
      const d = Math.hypot(mx - freqToX(loCutFreqDisp), my - LO_HANDLE_Y);
      if (d < bestDist) { bestDist = d; target = 'lo'; }
    }
    if (hiCutOn) {
      const d = Math.hypot(mx - freqToX(hiCutFreqDisp), my - qToY(hiCutQDisp));
      if (d < bestDist) { bestDist = d; target = 'hi'; }
    }

    if (target) {
      dragging       = true;
      dragTarget     = target;
      dragStartCX    = e.clientX;
      dragStartCY    = e.clientY;
      dragStartLoFreq = loCutFreqDisp;
      dragStartHiFreq = hiCutFreqDisp;
      dragStartQ      = hiCutQDisp;
      window.addEventListener('mousemove', onDragMove);
      window.addEventListener('mouseup',   onDragUp);
    }
  }

  function onDragMove(e) {
    if (!dragging) return;
    const rect  = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const dx = (e.clientX - dragStartCX) * scaleX;
    const dy = (e.clientY - dragStartCY) * scaleY;

    if (dragTarget === 'lo') {
      // 1a: Lo Cut handle — X only (no Q)
      const newFreq = Math.min(FMAX, Math.max(FMIN, xToFreq(freqToX(dragStartLoFreq) + dx)));
      send('loCutFreq', toNormalized(newFreq, FMIN, FMAX, 0.3));
    } else if (dragTarget === 'hi') {
      // 1a: Hi Cut handle — X=freq, Y=Q
      const newFreq = Math.min(FMAX, Math.max(FMIN, xToFreq(freqToX(dragStartHiFreq) + dx)));
      send('hiCutFreq', toNormalized(newFreq, FMIN, FMAX, 0.3));
      const newQ = Math.min(QMAX, Math.max(QMIN, yToQ(qToY(dragStartQ) + dy)));
      send('hiCutQ', toNormalized(newQ, QMIN, QMAX, 0.5));
    }
  }

  function onDragUp() {
    dragging   = false;
    dragTarget = null;
    draw();
    window.removeEventListener('mousemove', onDragMove);
    window.removeEventListener('mouseup',   onDragUp);
  }

  // ── Mouse: hover ───────────────────────────────────────────
  function onCanvasMouseMove(e) {
    if (dragging) return;
    const rect  = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const mx = (e.clientX - rect.left) * scaleX;
    const my = (e.clientY - rect.top)  * scaleY;

    let newHover = null;
    if (loCutOn  && Math.hypot(mx - freqToX(loCutFreqDisp), my - LO_HANDLE_Y)            < 12) newHover = 'lo';
    if (hiCutOn  && Math.hypot(mx - freqToX(hiCutFreqDisp), my - qToY(hiCutQDisp))        < 12) newHover = 'hi';

    if (newHover !== hoverTarget) { hoverTarget = newHover; draw(); }
  }

  function onCanvasMouseLeave() {
    if (hoverTarget !== null) { hoverTarget = null; draw(); }
  }

  // ── NeuNumber handlers ─────────────────────────────────────
  function onLoFreqChange(e) {
    const v = Math.min(FMAX, Math.max(FMIN, e.detail.value));
    send('loCutFreq', toNormalized(v, FMIN, FMAX, 0.3));
  }
  function onHiFreqChange(e) {
    const v = Math.min(FMAX, Math.max(FMIN, e.detail.value));
    send('hiCutFreq', toNormalized(v, FMIN, FMAX, 0.3));
  }
  function onQChange(e) {
    const v = Math.min(QMAX, Math.max(QMIN, e.detail.value));
    send('hiCutQ', toNormalized(v, QMIN, QMAX, 0.5));
  }
</script>

<div class="fg-wrap">

  <!-- ── Toggles ──────────────────────────────────────────── -->
  <div class="toggles">
    <NeuButton label="Lo Cut" active={loCutOn}
      on:change={e => send('loCutEnabled', e.detail.active ? 1 : 0)}
    />
    <NeuButton label="Hi Cut" active={hiCutOn}
      on:change={e => send('hiCutEnabled', e.detail.active ? 1 : 0)}
    />
  </div>

  <!-- ── Canvas ───────────────────────────────────────────── -->
  <div class="canvas-shell">
    <canvas
      bind:this={canvas}
      width={CW} height={CH}
      class="graph-canvas"
      class:dragging
      role="img"
      aria-label="Input filter frequency response graph"
      on:mousedown={onCanvasMouseDown}
      on:mousemove={onCanvasMouseMove}
      on:mouseleave={onCanvasMouseLeave}
    ></canvas>
  </div>

  <!-- ── Readouts — independent for each active filter ─────── -->
  <div class="controls-row">
    {#if loCutOn}
      <NeuNumber
        label="LO FREQ"
        value={Math.round(loCutFreqDisp)}
        min={50} max={18000} step={1} unit=" Hz" decimals={0}
        on:change={onLoFreqChange}
      />
    {/if}
    {#if hiCutOn}
      <NeuNumber
        label="HI FREQ"
        value={Math.round(hiCutFreqDisp)}
        min={50} max={18000} step={1} unit=" Hz" decimals={0}
        on:change={onHiFreqChange}
      />
      <NeuNumber
        label="WIDTH"
        value={+hiCutQDisp.toFixed(2)}
        min={0.5} max={9} step={0.01} decimals={2}
        on:change={onQChange}
      />
    {/if}
  </div>

</div>

<style>
  .fg-wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
    width: 100%;
  }

  .toggles {
    display: flex;
    gap: 12px;
    justify-content: center;
  }

  .canvas-shell {
    width: 100%;
    border-radius: 10px;
    overflow: hidden;
    box-shadow:
      inset 4px 4px 10px rgba(140, 120, 90, 0.35),
      inset -3px -3px 7px rgba(255, 250, 240, 0.6);
    background: #1a1a1a;
    line-height: 0;
  }

  .graph-canvas {
    display: block;
    width: 100%;
    height: auto;
    cursor: crosshair;
    border-radius: 10px;
  }

  .graph-canvas.dragging { cursor: grabbing; }

  .controls-row {
    display: flex;
    justify-content: center;
    gap: 12px;
    flex-wrap: wrap;
  }
</style>
