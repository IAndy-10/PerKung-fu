<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  const dispatch = createEventDispatcher<{ change: number }>();

  export let label: string   = '';
  export let value: number   = 0.5;  // 0..1 normalized
  export let display: string = '';

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
    dispatch('change', Math.max(0, Math.min(1, startV + dy / 200)));
  }

  function onMouseUp() {
    window.removeEventListener('mousemove', onMouseMove);
    window.removeEventListener('mouseup',  onMouseUp);
  }

  function onWheel(e: WheelEvent) {
    e.preventDefault();
    dispatch('change', Math.max(0, Math.min(1, value - e.deltaY * 0.001)));
  }
</script>

<div class="numf">
  <div class="numf-label">{label}</div>
  <div class="numf-pill" role="spinbutton" aria-valuenow={value} aria-valuemin={0} aria-valuemax={1} on:mousedown={onMouseDown} on:wheel={onWheel}>
    {display}
  </div>
</div>

<style>
  .numf { display: flex; flex-direction: column; gap: 6px; min-width: 0; }

  .numf-label {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.52rem; font-weight: 400; letter-spacing: 0.26em;
    text-transform: uppercase; color: var(--text-lo);
    padding-left: 10px;
  }
  .numf-pill {
    background: var(--glass-bg-hi);
    border: 1px solid var(--glass-border);
    border-radius: 8px; padding: 6px 12px;
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.66rem; font-weight: 400; color: var(--accent);
    letter-spacing: 0.02em;
    cursor: ns-resize; user-select: none;
    text-align: center; white-space: nowrap;
    transition: border-color 0.15s, background 0.15s;
    min-width: 60px;
    backdrop-filter: blur(8px);
    -webkit-backdrop-filter: blur(8px);
  }
  .numf-pill:hover {
    border-color: var(--accent-line);
    background: rgba(255,255,255,0.05);
  }
</style>
