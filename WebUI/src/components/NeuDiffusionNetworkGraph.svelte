<script lang="ts">
  import { onMount } from 'svelte';
  import { params, toNormalized } from '../state/store';
  import { bridge } from '../bridge/bridge';

  // activeBand is the only prop — dims inactive node visually
  export let activeBand = 'low';  // 'low' | 'high'

  const { crossoverFreq, diffusion, damping, feedback, highFilterType } = params;

  const CW = 320, CH = 130;
  const MX = 8,   MY = 10;
  const FMIN = 50, FMAX = 20000;

  const N2_FMIN = 800, N2_FMAX = 18000;

  let canvas;
  let dragging   = false;
  let activeNode = 0;
  let dragStartClientX, dragStartClientY;
  let dragStart  = {};
  let dragValues = null;  // non-null during drag, used by draw() for immediate feedback

  // ── Send helper ─────────────────────────────────────────────
  function send(id, value) {
    params[id].set(value);
    bridge.sendParameterChange(id, value);
  }

  // ── Coordinate conversions ──────────────────────────────────
  function freqToX(f) {
    return MX + (CW - 2 * MX) * Math.log10(f / FMIN) / Math.log10(FMAX / FMIN);
  }
  function xToFreq(x) {
    return FMIN * Math.pow(FMAX / FMIN, (x - MX) / (CW - 2 * MX));
  }

  // Node 1 (crossoverFreq): JUCE APVTS range 200–8000 Hz, skew=0.5
  // norm → display: hz = 200 + 7800 * norm²
  // display → norm: norm = sqrt((hz - 200) / 7800)
  function n1ToHz(norm) { return 200 + 7800 * norm * norm; }
  function hzToN1(hz)   { return Math.sqrt(Math.max(0, Math.min(1, (hz - 200) / 7800))); }

  // Node 2 (damping): visual log mapping 800–18000 Hz (damping stored 0–1)
  function n2ToFreq(p) { return N2_FMIN * Math.pow(N2_FMAX / N2_FMIN, p); }
  function freqToN2(f) { return Math.log(f / N2_FMIN) / Math.log(N2_FMAX / N2_FMIN); }

  function pToY(p) { return MY + (1 - p) * (CH - 2 * MY); }
  function yToP(y) { return Math.max(0, Math.min(1, 1 - (y - MY) / (CH - 2 * MY))); }

  // ── Current display values (drag takes priority over stores) ──
  function getVals() {
    return dragValues ?? {
      crossoverFreq:  $crossoverFreq,
      diffusion:      $diffusion,
      damping:        $damping,
      feedback:       $feedback,
      highFilterType: $highFilterType > 0.5,
    };
  }

  function getN1(v) {
    const f = n1ToHz(v.crossoverFreq);
    return { x: freqToX(f), y: pToY(v.diffusion), f };
  }
  function getN2(v) {
    const f = n2ToFreq(v.damping);
    return { x: freqToX(f), y: pToY(v.feedback), f };
  }

  function specMag(f, v) {
    const f1 = n1ToHz(v.crossoverFreq);
    const f2 = n2ToFreq(v.damping);
    if (f1 >= f2) return (v.diffusion + v.feedback) * 0.5;
    const t = Math.max(0, Math.min(1, Math.log(f / f1) / Math.log(f2 / f1)));
    const s = t * t * (3 - 2 * t);
    return v.diffusion * (1 - s) + v.feedback * s;
  }

  // ── Draw ────────────────────────────────────────────────────
  function draw() {
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const v   = getVals();
    const n1  = getN1(v);
    const n2  = getN2(v);

    // 3d: Axis labels — drawn first so they sit behind the curve
    ctx.font = '7px "DM Sans", sans-serif';
    ctx.fillStyle = 'rgba(200,200,200,0.25)';

    // X-axis: frequency markers
    ctx.textAlign = 'center';
    ctx.textBaseline = 'bottom';
    for (const [f, lbl] of [[200, '200'], [1000, '1k'], [4000, '4k'], [8000, '8k']]) {
      ctx.fillText(lbl, freqToX(f), CH - 1);
    }

    // Y-axis: + at top, − at bottom
    ctx.textAlign = 'left';
    ctx.textBaseline = 'top';
    ctx.fillText('+', MX, MY);
    ctx.textBaseline = 'bottom';
    ctx.fillText('−', MX, CH - MY);

    const n1Alpha = activeBand === 'high' ? 0.4 : 1.0;
    const n2Alpha = activeBand === 'low'  ? 0.4 : 1.0;

    // Background
    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, CW, CH);

    // Grid
    ctx.lineWidth = 0.5;
    ctx.strokeStyle = 'rgba(255,255,255,0.06)';
    for (const gf of [100, 200, 500, 1000, 2000, 5000, 10000]) {
      const gx = freqToX(gf);
      ctx.beginPath(); ctx.moveTo(gx, 0); ctx.lineTo(gx, CH); ctx.stroke();
    }
    ctx.strokeStyle = 'rgba(255,255,255,0.04)';
    ctx.beginPath(); ctx.moveTo(0, CH / 2); ctx.lineTo(CW, CH / 2); ctx.stroke();

    // Spectral envelope
    const pts = [];
    for (let px = MX; px <= CW - MX; px++) {
      pts.push([px, pToY(specMag(xToFreq(px), v))]);
    }
    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.lineTo(CW - MX, CH); ctx.lineTo(MX, CH); ctx.closePath();
    ctx.fillStyle = 'rgba(0,200,180,0.07)'; ctx.fill();

    ctx.beginPath();
    pts.forEach(([px, py], i) => i === 0 ? ctx.moveTo(px, py) : ctx.lineTo(px, py));
    ctx.strokeStyle = '#00c8b4'; ctx.lineWidth = 1.5; ctx.lineJoin = 'round'; ctx.stroke();

    // ── Node 1 (teal) ──────────────────────────────────────
    ctx.globalAlpha = n1Alpha;

    ctx.setLineDash([2, 3]); ctx.lineWidth = 0.7;
    ctx.beginPath(); ctx.moveTo(n1.x, MY); ctx.lineTo(n1.x, CH - MY);
    ctx.strokeStyle = 'rgba(0,200,180,0.25)'; ctx.stroke();
    ctx.setLineDash([]);

    ctx.beginPath(); ctx.arc(n1.x, n1.y, 9, 0, Math.PI * 2);
    ctx.fillStyle = (dragging && activeNode === 1) ? '#20e8d4' : '#00c8b4'; ctx.fill();
    ctx.beginPath(); ctx.arc(n1.x, n1.y, 9, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(150,240,220,0.45)'; ctx.lineWidth = 1.2; ctx.stroke();
    ctx.fillStyle = '#0e1f1e';
    ctx.font = 'bold 9px "DM Sans", sans-serif';
    ctx.textAlign = 'center'; ctx.textBaseline = 'middle';
    ctx.fillText('1', n1.x, n1.y);

    // ── Node 2 (orange) ─────────────────────────────────────
    ctx.globalAlpha = n2Alpha;

    ctx.setLineDash([2, 3]); ctx.lineWidth = 0.7;
    ctx.beginPath(); ctx.moveTo(n2.x, MY); ctx.lineTo(n2.x, CH - MY);
    ctx.strokeStyle = v.highFilterType ? 'rgba(245,166,35,0.10)' : 'rgba(245,166,35,0.25)';
    ctx.stroke(); ctx.setLineDash([]);

    const n2Color = v.highFilterType
      ? '#b8924a'
      : (dragging && activeNode === 2) ? '#ffb940' : '#f5a623';
    ctx.beginPath(); ctx.arc(n2.x, n2.y, 9, 0, Math.PI * 2);
    ctx.fillStyle = n2Color; ctx.fill();
    ctx.beginPath(); ctx.arc(n2.x, n2.y, 9, 0, Math.PI * 2);
    ctx.strokeStyle = v.highFilterType ? 'rgba(200,180,130,0.3)' : 'rgba(255,220,100,0.45)';
    ctx.lineWidth = 1.2; ctx.stroke();
    ctx.fillStyle = '#231700';
    ctx.fillText('2', n2.x, n2.y);

    if (v.highFilterType) {
      ctx.font = '7px "DM Sans", sans-serif';
      ctx.fillStyle = 'rgba(245,180,80,0.55)';
      ctx.textBaseline = 'bottom';
      ctx.fillText('SH', n2.x, n2.y - 10);
    }

    ctx.globalAlpha = 1.0;
    ctx.textBaseline = 'alphabetic';
  }

  $: if (canvas) { $crossoverFreq; $diffusion; $damping; $feedback; $highFilterType; activeBand; draw(); }
  onMount(() => draw());

  // ── Mouse interaction ───────────────────────────────────────
  function onCanvasMouseDown(e) {
    const rect  = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const mx = (e.clientX - rect.left) * scaleX;
    const my = (e.clientY - rect.top)  * scaleY;

    const v  = getVals();
    const n1 = getN1(v), n2 = getN2(v);
    const d1 = Math.hypot(mx - n1.x, my - n1.y);
    const d2 = Math.hypot(mx - n2.x, my - n2.y);
    const HIT = 15;

    if (d1 < HIT || d2 < HIT) {
      activeNode       = (d1 <= d2 && d1 < HIT) ? 1 : 2;
      dragging         = true;
      dragStartClientX = e.clientX;
      dragStartClientY = e.clientY;
      dragStart        = { ...v };
      dragValues       = { ...v };
      window.addEventListener('mousemove', onDragMove);
      window.addEventListener('mouseup',   onDragUp);
    }
  }

  function onDragMove(e) {
    if (!dragging) return;
    const rect  = canvas.getBoundingClientRect();
    const scaleX = CW / rect.width;
    const scaleY = CH / rect.height;
    const dx = (e.clientX - dragStartClientX) * scaleX;
    const dy = (e.clientY - dragStartClientY) * scaleY;

    dragValues = { ...dragStart };

    if (activeNode === 1) {
      const startX  = freqToX(n1ToHz(dragStart.crossoverFreq));
      const newHz   = Math.min(8000, Math.max(200, xToFreq(startX + dx)));
      const newNorm = hzToN1(newHz);
      dragValues.crossoverFreq = newNorm;
      dragValues.diffusion     = yToP(pToY(dragStart.diffusion) + dy);
      send('crossoverFreq', newNorm);
      send('diffusion',     dragValues.diffusion);
    } else {
      const startX  = freqToX(n2ToFreq(dragStart.damping));
      const newHz   = Math.min(N2_FMAX, Math.max(N2_FMIN, xToFreq(startX + dx)));
      const newNorm = freqToN2(newHz);
      dragValues.damping = newNorm;
      send('damping', newNorm);
      if (!dragStart.highFilterType) {
        dragValues.feedback = yToP(pToY(dragStart.feedback) + dy);
        send('feedback', dragValues.feedback);
      }
    }

    draw();
  }

  function onDragUp() {
    dragging   = false;
    dragValues = null;
    draw();
    window.removeEventListener('mousemove', onDragMove);
    window.removeEventListener('mouseup',   onDragUp);
  }
</script>

<div class="dng-wrap">
  <div class="canvas-shell">
    <canvas
      bind:this={canvas}
      width={CW} height={CH}
      class="graph-canvas"
      class:dragging
      on:mousedown={onCanvasMouseDown}
    ></canvas>
  </div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300;400&display=swap');

  .dng-wrap {
    display: flex;
    flex-direction: column;
    user-select: none;
    width: 100%;
  }

  .canvas-shell {
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
</style>
