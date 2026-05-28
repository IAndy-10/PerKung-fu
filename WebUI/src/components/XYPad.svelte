<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  const dispatch = createEventDispatcher<{ change: { x: number; y: number } }>();

  export let x: number       = 0.5;
  export let y: number       = 0.5;
  export let xDisplay: string = '';
  export let yDisplay: string = '';

  let padEl: HTMLElement;

  function onMouseDown(e: MouseEvent) {
    e.preventDefault();
    update(e.clientX, e.clientY);
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup',  onMouseUp);
  }

  function update(clientX: number, clientY: number) {
    const rect = padEl.getBoundingClientRect();
    dispatch('change', {
      x: Math.max(0, Math.min(1, (clientX - rect.left)  / rect.width)),
      y: Math.max(0, Math.min(1, (clientY - rect.top)   / rect.height)),
    });
  }

  function onMouseMove(e: MouseEvent) { update(e.clientX, e.clientY); }

  function onMouseUp() {
    window.removeEventListener('mousemove', onMouseMove);
    window.removeEventListener('mouseup',  onMouseUp);
  }
</script>

<div class="xy-pad" bind:this={padEl} on:mousedown={onMouseDown}>
  <div class="xy-hline"></div>
  <div class="xy-vline"></div>
  <div class="xy-corner tl"></div>
  <div class="xy-corner tr"></div>
  <div class="xy-corner bl"></div>
  <div class="xy-corner br"></div>
  <div class="xy-point" style="left: {x * 100}%; top: {y * 100}%"></div>
  <div class="xy-readout">
    <span>{xDisplay}</span>
    <span>{yDisplay}</span>
  </div>
</div>

<style>
  .xy-pad {
    position: relative;
    width: 100%; aspect-ratio: 1; max-width: 180px;
    background: linear-gradient(135deg, #0a0810, #14111d);
    border: 1px solid rgba(255,255,255,0.05);
    border-radius: 10px;
    box-shadow: inset 0 2px 8px rgba(0,0,0,0.5);
    cursor: crosshair; user-select: none; margin-top: 6px;
  }
  .xy-hline {
    position: absolute; left: 0; right: 0; top: 50%;
    height: 1px; background: rgba(255,255,255,0.04);
  }
  .xy-vline {
    position: absolute; top: 0; bottom: 0; left: 50%;
    width: 1px; background: rgba(255,255,255,0.04);
  }
  .xy-corner {
    position: absolute; width: 8px; height: 8px;
    border: 1px solid rgba(255,255,255,0.08);
  }
  .xy-corner.tl { top: 4px; left: 4px; border-right: none; border-bottom: none; }
  .xy-corner.tr { top: 4px; right: 4px; border-left: none; border-bottom: none; }
  .xy-corner.bl { bottom: 4px; left: 4px; border-right: none; border-top: none; }
  .xy-corner.br { bottom: 4px; right: 4px; border-left: none; border-top: none; }
  .xy-point {
    position: absolute; width: 12px; height: 12px; border-radius: 50%;
    background: var(--accent);
    box-shadow: 0 0 12px var(--accent-glow), 0 0 24px var(--accent-dim);
    transform: translate(-50%, -50%); pointer-events: none;
  }
  .xy-readout {
    position: absolute; bottom: 6px; left: 10px; right: 10px;
    display: flex; justify-content: space-between;
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.5rem; letter-spacing: 0.16em;
    text-transform: uppercase; color: var(--accent);
    pointer-events: none;
  }
</style>
