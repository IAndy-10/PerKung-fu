<script>
  import { createEventDispatcher } from 'svelte';

  export let x = 0.5;
  export let y = 0.5;
  export let labelX = 'X';
  export let labelY = 'Y';

  const dispatch = createEventDispatcher();

  let padEl;
  let dragging = false;

  function getPos(e) {
    const rect = padEl.getBoundingClientRect();
    return {
      x: Math.min(1, Math.max(0, (e.clientX - rect.left) / rect.width)),
      y: Math.min(1, Math.max(0, 1 - (e.clientY - rect.top) / rect.height)),
    };
  }

  function onDown(e) {
    dragging = true;
    const p = getPos(e); x = p.x; y = p.y;
    dispatch('change', { x, y });
    window.addEventListener('mousemove', onMove);
    window.addEventListener('mouseup', onUp);
  }
  function onMove(e) {
    if (!dragging) return;
    const p = getPos(e); x = p.x; y = p.y;
    dispatch('change', { x, y });
  }
  function onUp() {
    dragging = false;
    dispatch('change', { x, y });
    window.removeEventListener('mousemove', onMove);
    window.removeEventListener('mouseup', onUp);
  }
</script>

<div class="wrap">
  <div class="top-lbl"><span>{labelX}</span><span>{labelY}</span></div>

  <div
    class="pad"
    class:active={dragging}
    bind:this={padEl}
    on:mousedown={onDown}
    role="slider"
    tabindex="0"
  >
    <div class="bowl">
      <div class="sphere large" style="
        left: {x * 60 + 10}%;
        top: {(1 - y) * 60 + 10}%;
      "></div>
      <div class="sphere small" style="
        left: {x * 40 + 30}%;
        top: {(1 - y) * 40 + 44}%;
      "></div>
    </div>
  </div>

  <div class="vals">
    <span>{(x * 100).toFixed(0)}</span>
    <span>{(y * 100).toFixed(0)}</span>
  </div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300&display=swap');

  .wrap {
    display: flex;
    flex-direction: column;
    gap: 8px;
    align-items: center;
    user-select: none;
  }

  .top-lbl {
    display: flex;
    justify-content: space-between;
    width: 100%;
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.6);
  }

  .pad {
    width: 130px;
    height: 130px;
    border-radius: 50%;
    background: #e8e0d4;
    box-shadow:
      8px 8px 18px rgba(155,135,105,0.45),
      -6px -6px 15px rgba(255,252,244,0.92);
    cursor: crosshair;
    transition: box-shadow 0.15s;
    overflow: hidden;
    position: relative;
  }

  .pad.active {
    box-shadow:
      4px 4px 10px rgba(155,135,105,0.4),
      -3px -3px 8px rgba(255,252,244,0.85);
  }

  .bowl {
    position: absolute;
    inset: 14px;
    border-radius: 50%;
    background: #e2d9cb;
    box-shadow:
      inset 5px 5px 12px rgba(145,125,95,0.4),
      inset -4px -4px 10px rgba(255,250,238,0.75);
    overflow: hidden;
  }

  .sphere {
    position: absolute;
    border-radius: 50%;
    transform: translate(-50%, -50%);
    pointer-events: none;
    transition: left 0.04s, top 0.04s;
  }

  .sphere.large {
    width: 55%;
    height: 55%;
    background: radial-gradient(circle at 35% 28%, #f8f3ea, #d8cfc0);
    box-shadow:
      4px 5px 12px rgba(140,120,88,0.5),
      -2px -3px 8px rgba(255,252,242,0.85);
  }

  .sphere.small {
    width: 22%;
    height: 22%;
    background: radial-gradient(circle at 35% 30%, #f8f3ea, #ddd4c4);
    box-shadow:
      2px 3px 6px rgba(140,120,88,0.45),
      -1px -1px 4px rgba(255,252,242,0.8);
  }

  .vals {
    display: flex;
    justify-content: space-between;
    width: 100%;
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 200;
    color: rgba(150, 120, 80, 0.6);
    letter-spacing: 0.08em;
  }
</style>
