<script>
  import { onMount } from 'svelte';

  export let cols = 6;
  export let rows = 10;

  // Each cell: { active, intensity, flickerOffset }
  let cells = [];

  function initCells() {
    cells = Array.from({ length: rows * cols }, (_, i) => ({
      active: Math.random() > 0.62,
      intensity: 0.4 + Math.random() * 0.6,
      flickerOffset: Math.random() * Math.PI * 2,
      size: Math.random() > 0.5 ? 'tall' : 'wide',
    }));
  }

  let t = 0;
  let animFrame;

  onMount(() => {
    initCells();

    function loop() {
      t += 0.012;
      // Slowly toggle some cells and pulse intensity
      cells = cells.map((c, i) => {
        const flicker = 0.6 + Math.sin(t * 1.1 + c.flickerOffset) * 0.4;
        // Rare random toggles
        const toggle = Math.random() > 0.9985 ? !c.active : c.active;
        return { ...c, active: toggle, intensity: c.intensity * 0.95 + flicker * 0.05 };
      });
      animFrame = requestAnimationFrame(loop);
    }

    loop();
    return () => cancelAnimationFrame(animFrame);
  });
</script>

<div class="matrix-wrap">
  <div class="label-top">VU MATRIX</div>
  <div class="panel">
    <div class="grid" style="grid-template-columns: repeat({cols}, 1fr); grid-template-rows: repeat({rows}, 1fr);">
      {#each cells as cell, i}
        <div
          class="cell {cell.size}"
          class:active={cell.active}
          style="--intensity: {cell.intensity}; --row: {Math.floor(i / cols)}"
        ></div>
      {/each}
    </div>
    <!-- Frosted overlay -->
    <div class="frost"></div>
  </div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=Jost:wght@200;300&display=swap');

  .matrix-wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
  }

  .label-top {
    font-family: 'Jost', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.3em;
    text-transform: uppercase;
    color: rgba(180, 170, 210, 0.6);
  }

  .panel {
    position: relative;
    width: 130px;
    height: 220px;
    background: linear-gradient(
      to bottom,
      rgba(230, 225, 255, 0.1) 0%,
      rgba(180, 160, 240, 0.08) 60%,
      rgba(160, 140, 230, 0.12) 100%
    );
    border: 1px solid rgba(255,255,255,0.18);
    border-radius: 12px;
    padding: 12px 10px;
    backdrop-filter: blur(12px);
    box-shadow:
      0 8px 32px rgba(0,0,0,0.25),
      inset 0 1px 0 rgba(255,255,255,0.2);
    overflow: hidden;
  }

  .grid {
    display: grid;
    gap: 4px;
    width: 100%;
    height: 100%;
  }

  .cell {
    border-radius: 3px;
    border: 1px solid rgba(255,255,255,0.1);
    background: rgba(255,255,255,0.04);
    transition: background 0.3s, box-shadow 0.3s;
  }

  .cell.active {
    background: rgba(255, calc(80 + var(--row, 0) * 8), 30, calc(var(--intensity) * 0.9));
    border-color: rgba(255, 160, 80, 0.4);
    box-shadow:
      0 0 6px rgba(255, 90, 30, calc(var(--intensity) * 0.7)),
      0 0 12px rgba(255, 60, 10, calc(var(--intensity) * 0.3));
  }

  .frost {
    position: absolute;
    inset: 0;
    border-radius: 12px;
    background: linear-gradient(
      to bottom,
      rgba(255,255,255,0.04) 0%,
      transparent 40%,
      rgba(160,140,230,0.07) 100%
    );
    pointer-events: none;
  }
</style>
