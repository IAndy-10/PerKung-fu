<script>
  export let label = 'FREQ';
  export let value = 0.5; // 0–1
  export let min = 0;
  export let max = 100;
  export let unit = '';

  let dragging = false;
  let trackEl;
  let startY = 0;
  let startVal = value;

  $: displayVal = (min + value * (max - min)).toFixed(0);

  function onMouseDown(e) {
    dragging = true;
    startY = e.clientY;
    startVal = value;
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup', onMouseUp);
  }

  function onMouseMove(e) {
    if (!dragging || !trackEl) return;
    const rect = trackEl.getBoundingClientRect();
    const rel = 1 - (e.clientY - rect.top) / rect.height;
    value = Math.min(1, Math.max(0, rel));
  }

  function onMouseUp() {
    dragging = false;
    window.removeEventListener('mousemove', onMouseMove);
    window.removeEventListener('mouseup', onMouseUp);
  }

  function onWheel(e) {
    e.preventDefault();
    value = Math.min(1, Math.max(0, value - e.deltaY * 0.001));
  }
</script>

<div class="slider-wrap">
  <div class="label">{label}</div>

  <div class="track" bind:this={trackEl} on:mousedown={onMouseDown} on:wheel={onWheel}>
    <!-- Fill -->
    <div class="fill" style="height: {value * 100}%"></div>

    <!-- Thumb -->
    <div
      class="thumb"
      class:active={dragging}
      style="bottom: calc({value * 100}% - 10px)"
    >
      <div class="thumb-line"></div>
    </div>

    <!-- Tick marks -->
    {#each [0,0.25,0.5,0.75,1] as tick}
      <div class="tick" style="bottom: {tick * 100}%"></div>
    {/each}
  </div>

  <div class="value">{displayVal}{unit}</div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=Jost:wght@200;300;400&display=swap');

  .slider-wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 8px;
    user-select: none;
  }

  .label {
    font-family: 'Jost', sans-serif;
    font-size: 0.6rem;
    font-weight: 300;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: rgba(180, 170, 210, 0.7);
  }

  .track {
    position: relative;
    width: 18px;
    height: 100px;
    background: rgba(255,255,255,0.05);
    border: 1px solid rgba(255,255,255,0.12);
    border-radius: 10px;
    cursor: ns-resize;
    backdrop-filter: blur(4px);
    overflow: visible;
  }

  .fill {
    position: absolute;
    bottom: 0;
    left: 0;
    right: 0;
    border-radius: 10px;
    background: linear-gradient(to top, rgba(255,100,40,0.7), rgba(255,180,100,0.3));
    box-shadow: 0 0 10px rgba(255,90,30,0.4);
    transition: height 0.05s;
  }

  .thumb {
    position: absolute;
    left: 50%;
    transform: translateX(-50%);
    width: 28px;
    height: 14px;
    background: rgba(255,255,255,0.12);
    border: 1px solid rgba(255,255,255,0.3);
    border-radius: 4px;
    backdrop-filter: blur(6px);
    box-shadow: 0 2px 8px rgba(0,0,0,0.4);
    display: flex;
    align-items: center;
    justify-content: center;
    z-index: 2;
    cursor: grab;
    transition: box-shadow 0.1s;
  }

  .thumb.active {
    box-shadow: 0 2px 12px rgba(255,100,40,0.5);
    cursor: grabbing;
  }

  .thumb-line {
    width: 12px;
    height: 1px;
    background: rgba(255,255,255,0.6);
    border-radius: 1px;
  }

  .tick {
    position: absolute;
    right: -6px;
    width: 4px;
    height: 1px;
    background: rgba(255,255,255,0.2);
    transform: translateY(50%);
  }

  .value {
    font-family: 'Jost', sans-serif;
    font-size: 0.6rem;
    font-weight: 200;
    color: rgba(255, 150, 80, 0.75);
    letter-spacing: 0.1em;
  }
</style>
