<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  const dispatch = createEventDispatcher<{ change: number }>();

  export let label: string   = '';
  export let value: number   = 0.5;  // 0..1 normalized
  export let display: string = '';

  let trackEl: HTMLElement;

  function onMouseDown(e: MouseEvent) {
    e.preventDefault();
    update(e.clientY);
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup',  onMouseUp);
  }

  function update(clientY: number) {
    const rect = trackEl.getBoundingClientRect();
    dispatch('change', 1 - Math.max(0, Math.min(1, (clientY - rect.top) / rect.height)));
  }

  function onMouseMove(e: MouseEvent) { update(e.clientY); }

  function onMouseUp() {
    window.removeEventListener('mousemove', onMouseMove);
    window.removeEventListener('mouseup',  onMouseUp);
  }
</script>

<div class="vslider-wrap">
  <div class="vslider-label">{label}</div>
  <div class="vslider-track-wrap">
    <div class="vslider-track" bind:this={trackEl} on:mousedown={onMouseDown}>
      <div class="vslider-fill" style="height: {value * 100}%" />
    </div>
    <div class="vslider-handle" style="bottom: {value * 100}%" />
  </div>
  <div class="vslider-val">{display}</div>
</div>

<style>
  .vslider-wrap {
    display: flex; flex-direction: column; align-items: center;
    gap: 8px; user-select: none;
  }
  .vslider-label {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.54rem; font-weight: 400; letter-spacing: 0.26em;
    text-transform: uppercase; color: var(--text-lo);
  }
  .vslider-track-wrap {
    position: relative; width: 26px; height: 130px;
    display: flex; align-items: flex-end; justify-content: center;
  }
  .vslider-track {
    position: absolute; inset: 0;
    width: 10px; height: 100%;
    left: 50%; transform: translateX(-50%);
    background: rgba(255,255,255,0.022);
    border-radius: 6px;
    border: 1px solid var(--glass-border);
    overflow: hidden; cursor: ns-resize;
  }
  .vslider-fill {
    position: absolute; bottom: 0; left: 0; right: 0;
    background: linear-gradient(to top, var(--accent), rgba(255,182,72,0.55));
    box-shadow: 0 0 8px var(--accent-dim);
  }
  .vslider-handle {
    position: absolute;
    left: 50%; transform: translate(-50%, 50%);
    width: 18px; height: 6px; border-radius: 2px;
    background: rgba(232,226,248,0.85);
    box-shadow: 0 0 8px var(--accent-glow);
    pointer-events: none;
  }
  .vslider-val {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.6rem; font-weight: 400; color: var(--accent);
  }
</style>
