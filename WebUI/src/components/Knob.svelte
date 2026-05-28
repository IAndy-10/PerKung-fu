<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  const dispatch = createEventDispatcher<{ change: number }>();

  export let label: string    = '';
  export let value: number    = 0.5;   // 0..1 normalized
  export let display: string  = '';
  export let size: 'sm' | 'md' | 'lg' = 'md';
  export let showArc: boolean = true;

  // Arc geometry (constant)
  const R = 26, C = 32, arcLen = 270;
  const dashTotal = 2 * Math.PI * R;
  const fullDash  = (arcLen / 360) * dashTotal;

  $: valDash  = value * fullDash;
  $: rotation = -135 + value * 270;

  let startY = 0, startV = 0;

  function onMouseDown(e: MouseEvent) {
    e.preventDefault();
    startY = e.clientY;
    startV = value;
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup',  onMouseUp);
  }

  function onMouseMove(e: MouseEvent) {
    const dy = startY - e.clientY;
    dispatch('change', Math.max(0, Math.min(1, startV + dy / 140)));
  }

  function onMouseUp() {
    window.removeEventListener('mousemove', onMouseMove);
    window.removeEventListener('mouseup',  onMouseUp);
  }

  function onWheel(e: WheelEvent) {
    e.preventDefault();
    dispatch('change', Math.max(0, Math.min(1, value - e.deltaY * 0.001)));
  }

  function onDblClick() { dispatch('change', 0.5); }
</script>

<!-- svelte-ignore a11y-no-noninteractive-tabindex -->
<div class="knob-wrap">
  <div
    class="knob {size}"
    role="slider"
    tabindex="0"
    aria-valuenow={value}
    aria-valuemin={0}
    aria-valuemax={1}
    on:mousedown={onMouseDown}
    on:wheel={onWheel}
    on:dblclick={onDblClick}
  >
    {#if showArc}
      <svg class="knob-arc" viewBox="0 0 64 64">
        <circle cx={C} cy={C} r={R} fill="none"
          stroke="rgba(255,255,255,0.05)" stroke-width="2"
          stroke-dasharray="{fullDash} {dashTotal}" stroke-linecap="round"
          transform="rotate(135 32 32)" />
        <circle cx={C} cy={C} r={R} fill="none"
          stroke="var(--accent)" stroke-width="2"
          stroke-dasharray="{valDash} {dashTotal}" stroke-linecap="round"
          transform="rotate(135 32 32)"
          style="filter: drop-shadow(0 0 3px var(--accent-glow)); opacity: 0.9;" />
      </svg>
    {/if}
    <div class="knob-body" />
    <div class="knob-indicator" style="transform: rotate({rotation}deg)">
      <div class="knob-dot" />
    </div>
  </div>
  <div class="knob-label">{label}</div>
  <div class="knob-val">{display}</div>
</div>

<style>
  .knob-wrap {
    display: flex; flex-direction: column; align-items: center;
    gap: 10px; user-select: none; min-width: 64px;
  }
  .knob {
    position: relative;
    width: var(--knob-md); height: var(--knob-md);
    cursor: ns-resize; flex-shrink: 0;
  }
  .knob.lg { width: var(--knob-lg); height: var(--knob-lg); }
  .knob.sm { width: var(--knob-sm); height: var(--knob-sm); }

  .knob-body {
    position: absolute; inset: 12%;
    border-radius: 50%;
    background:
      radial-gradient(circle at 30% 30%, rgba(255,255,255,0.04), rgba(255,255,255,0.005) 70%),
      rgba(20, 18, 30, 0.55);
    border: 1px solid var(--glass-border);
    backdrop-filter: blur(8px);
    -webkit-backdrop-filter: blur(8px);
    box-shadow:
      inset 0 1px 1px rgba(255,255,255,0.04),
      inset 0 -1px 2px rgba(0,0,0,0.35);
    transition: border-color 0.15s;
  }
  .knob:hover .knob-body { border-color: var(--glass-border-hi); }

  .knob-arc       { position: absolute; inset: 0; pointer-events: none; }
  .knob-indicator { position: absolute; inset: 0; pointer-events: none; transition: transform 0.05s linear; }

  .knob-dot {
    position: absolute;
    left: 50%; top: 18%;
    width: 6px; height: 6px; border-radius: 50%;
    background: var(--accent);
    box-shadow: 0 0 6px var(--accent-glow), 0 0 14px var(--accent-dim);
    transform: translate(-50%, 0);
  }
  .knob.lg .knob-dot { width: 7px; height: 7px; }
  .knob.sm .knob-dot { width: 5px; height: 5px; }

  .knob-label {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.56rem; font-weight: 400; letter-spacing: 0.26em;
    text-transform: uppercase; color: var(--text-lo); white-space: nowrap;
  }
  .knob-val {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.62rem; font-weight: 400; letter-spacing: 0.04em;
    color: var(--accent); white-space: nowrap; min-height: 0.8rem;
  }
</style>
