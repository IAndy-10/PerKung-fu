<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  import { tweened } from 'svelte/motion';
  import { linear } from 'svelte/easing';
  import { tooltipStore } from '../stores/tooltip';

  export let label        = 'GAIN';
  export let value        = 0.5;     // normalized 0–1
  export let min          = 0;
  export let max          = 100;
  export let unit         = '';
  // 2d: dblclick resets to this normalized value (defaults to midpoint 0.5)
  export let defaultValue: number | null = null;
  // 3a: 'normal' = 64px, 'large' = 83px (30% bigger)
  export let size: 'normal' | 'large' = 'normal';
  // 3b: optional custom display formatter — receives the de-normalized value
  export let formatFn: ((v: number) => string) | null = null;
  // 3a: show larger value text
  export let prominent    = false;
  // 3a: show teal tick at 12 o'clock reference
  export let showTick     = false;

  const dispatch = createEventDispatcher();

  let dragging = false;
  let startY   = 0;
  let startVal = value;

  const MIN_ANGLE = -135;
  const MAX_ANGLE =  135;

  // 4a: tweened display — instant during drag, 80ms on external update
  const tween = tweened(value, { duration: 80, easing: linear });
  $: tween.set(value, { duration: dragging ? 0 : 80 });

  $: angle      = MIN_ANGLE + $tween * (MAX_ANGLE - MIN_ANGLE);
  $: rawDisplay = min + $tween * (max - min);
  $: displayVal = formatFn ? formatFn(rawDisplay) : rawDisplay.toFixed(1);

  function onMouseDown(e) {
    dragging = true;
    startY   = e.clientY;
    startVal = value;
    window.addEventListener('mousemove', onMove);
    window.addEventListener('mouseup',   onUp);
  }
  function onMove(e) {
    if (!dragging) return;
    value = Math.min(1, Math.max(0, startVal + (startY - e.clientY) / 120));
    dispatch('change', { value });
  }
  function onUp() {
    dragging = false;
    dispatch('change', { value });
    window.removeEventListener('mousemove', onMove);
    window.removeEventListener('mouseup',   onUp);
  }
  function onWheel(e) {
    e.preventDefault();
    value = Math.min(1, Math.max(0, value - e.deltaY * 0.001));
    dispatch('change', { value });
  }

  // 2d: double-click resets to defaultValue (or 0.5 if not set)
  function onDblClick(e) {
    e.preventDefault();
    const def = defaultValue !== null ? defaultValue : 0.5;
    value = def;
    dispatch('change', { value });
  }

  // ── Tooltip (4b) ───────────────────────────────────────────
  let ttTimer;
  function onMouseEnter(e) {
    ttTimer = setTimeout(() => {
      tooltipStore.set({
        visible: true,
        x: e.clientX + 12,
        y: e.clientY - 52,
        name: label,
        value: `${displayVal}${unit}`,
        range: `${min} – ${max}${unit}`,
      });
    }, 600);
  }
  function onMouseLeave() {
    clearTimeout(ttTimer);
    tooltipStore.update(s => ({ ...s, visible: false }));
  }
  function onMouseMove(e) {
    tooltipStore.update(s => s.visible
      ? { ...s, x: e.clientX + 12, y: e.clientY - 52 }
      : s
    );
  }

  // ── Context menu (2e) ──────────────────────────────────────
  let ctxVisible = false;
  let ctxX = 0, ctxY = 0;

  function onContextMenu(e) {
    e.preventDefault();
    ctxX = e.clientX; ctxY = e.clientY;
    ctxVisible = true;
    setTimeout(() => window.addEventListener('mousedown', closeCtx, { once: true }), 0);
  }
  function closeCtx() { ctxVisible = false; }

  function ctxSetDefault() {
    const def = defaultValue !== null ? defaultValue : 0.5;
    value = def;
    dispatch('change', { value });
    closeCtx();
  }
  async function ctxCopy() {
    await navigator.clipboard.writeText(`${displayVal}${unit}`).catch(() => {});
    closeCtx();
  }
  async function ctxPaste() {
    try {
      const text = await navigator.clipboard.readText();
      const parsed = parseFloat(text);
      if (!isNaN(parsed)) {
        value = Math.min(1, Math.max(0, (parsed - min) / (max - min)));
        dispatch('change', { value });
      }
    } catch {}
    closeCtx();
  }
</script>

<div
  class="wrap"
  on:mouseenter={onMouseEnter}
  on:mouseleave={onMouseLeave}
  on:mousemove={onMouseMove}
  on:contextmenu={onContextMenu}
>
  <div
    class="knob"
    class:pressed={dragging}
    class:large={size === 'large'}
    on:mousedown={onMouseDown}
    on:wheel|passive={onWheel}
    on:dblclick={onDblClick}
    role="slider"
    tabindex="0"
    aria-label="{label}: {displayVal}{unit}"
    aria-valuenow={rawDisplay}
    aria-valuemin={min}
    aria-valuemax={max}
  >
    {#if showTick}
      <!-- 3a: fixed teal tick at 12 o'clock outside the rotating inner -->
      <svg class="arc tick-layer" viewBox="0 0 64 64" aria-hidden="true">
        <line x1="32" y1="4" x2="32" y2="10"
          stroke="#00c8b4" stroke-width="2" stroke-linecap="round" opacity="0.6"/>
      </svg>
    {/if}

    <div class="inner" style="transform: rotate({angle}deg)">
      <div class="marker"></div>
    </div>

    <svg class="arc" viewBox="0 0 64 64" aria-hidden="true">
      <circle cx="32" cy="32" r="26"
        fill="none"
        stroke="rgba(180,165,145,0.18)"
        stroke-width="2"
        stroke-dasharray="130 41"
        stroke-dashoffset="10"
        stroke-linecap="round"
        transform="rotate(135 32 32)"
      />
      <circle cx="32" cy="32" r="26"
        fill="none"
        stroke="rgba(160,130,90,0.35)"
        stroke-width="2"
        stroke-dasharray="{value * 130} 171"
        stroke-dashoffset="10"
        stroke-linecap="round"
        transform="rotate(135 32 32)"
      />
    </svg>
  </div>

  <div class="label">{label}</div>
  <div class="val" class:val-prominent={prominent}>{displayVal}{unit}</div>

  <!-- Context menu -->
  {#if ctxVisible}
    <div class="ctx-menu" style="left:{ctxX}px;top:{ctxY}px;" role="menu">
      <button class="ctx-item" on:click={ctxSetDefault}>Set to default</button>
      <button class="ctx-item" on:click={ctxCopy}>Copy value</button>
      <button class="ctx-item" on:click={ctxPaste}>Paste value</button>
    </div>
  {/if}
</div>

<style>
  .wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 6px;
    user-select: none;
    position: relative;
  }

  .knob {
    position: relative;
    width: 64px;
    height: 64px;
    border-radius: 50%;
    background: #e8e0d4;
    box-shadow:
      6px 6px 14px rgba(160, 140, 110, 0.45),
      -5px -5px 12px rgba(255, 252, 245, 0.9);
    cursor: ns-resize;
    /* 4c: smooth hover shadow transition */
    transition: box-shadow 0.15s ease;
    display: flex;
    align-items: center;
    justify-content: center;
  }

  /* 3a: large variant */
  .knob.large {
    width: 83px;
    height: 83px;
  }

  /* 4c: hover state — brighter light shadow, slightly reduced dark shadow */
  .knob:hover:not(.pressed) {
    box-shadow:
      5px 5px 12px rgba(160, 140, 110, 0.38),
      -6px -6px 14px rgba(255, 252, 245, 0.97);
  }

  .knob.pressed {
    box-shadow:
      2px 2px 6px rgba(160, 140, 110, 0.5),
      -2px -2px 5px rgba(255, 252, 245, 0.8),
      inset 3px 3px 8px rgba(150, 130, 100, 0.25),
      inset -2px -2px 6px rgba(255, 252, 240, 0.7);
  }

  .arc {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    pointer-events: none;
  }

  .tick-layer {
    z-index: 1;
  }

  .inner {
    width: 40px;
    height: 40px;
    border-radius: 50%;
    background: radial-gradient(circle at 35% 30%, #f5efe6, #ddd4c4);
    box-shadow:
      3px 3px 8px rgba(150,130,100,0.4),
      -2px -2px 6px rgba(255,250,240,0.85);
    display: flex;
    align-items: flex-start;
    justify-content: center;
    padding-top: 5px;
    z-index: 2;
    position: relative;
  }

  /* Scale inner proportionally for large knob */
  .large .inner {
    width: 52px;
    height: 52px;
    padding-top: 7px;
  }

  .marker {
    width: 3px;
    height: 8px;
    border-radius: 2px;
    background: rgba(140, 110, 70, 0.6);
  }

  .label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.7);
  }

  .val {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 200;
    color: rgba(150, 120, 80, 0.65);
    letter-spacing: 0.08em;
  }

  /* 3a: larger value text for prominent controls */
  .val.val-prominent {
    font-size: 0.75rem;
    font-weight: 400;
    color: rgba(130, 100, 60, 0.82);
  }

  /* ── Context menu ─────────────────────────────────────── */
  .ctx-menu {
    position: fixed;
    z-index: 9998;
    background: #ede6da;
    border-radius: 10px;
    padding: 6px;
    display: flex;
    flex-direction: column;
    gap: 2px;
    box-shadow:
      8px 8px 20px rgba(155,135,105,0.38),
      -6px -6px 16px rgba(255,252,244,0.92);
    min-width: 120px;
  }

  .ctx-item {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.62rem;
    font-weight: 300;
    letter-spacing: 0.08em;
    color: rgba(120, 90, 55, 0.8);
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 6px 10px;
    text-align: left;
    cursor: pointer;
    transition: background 0.12s;
  }

  .ctx-item:hover {
    background: #e2d9cc;
    color: rgba(100, 72, 38, 0.92);
  }
</style>
