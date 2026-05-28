<script>
  export let label = 'GAIN';
  export let value = 0.5; // 0–1
  export let min = 0;
  export let max = 1;
  export let unit = '';

  let dragging = false;
  let startY = 0;
  let startVal = value;

  const MIN_ANGLE = -140;
  const MAX_ANGLE = 140;
  $: angle = MIN_ANGLE + value * (MAX_ANGLE - MIN_ANGLE);
  $: displayVal = (min + value * (max - min)).toFixed(1);

  function onMouseDown(e) {
    dragging = true;
    startY = e.clientY;
    startVal = value;
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup', onMouseUp);
  }

  function onMouseMove(e) {
    if (!dragging) return;
    const delta = (startY - e.clientY) / 140;
    value = Math.min(1, Math.max(0, startVal + delta));
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

<div class="knob-wrap">
  <div class="label">{label}</div>

  <div
    class="knob"
    class:active={dragging}
    on:mousedown={onMouseDown}
    on:wheel={onWheel}
    role="slider"
    aria-valuenow={displayVal}
    aria-valuemin={min}
    aria-valuemax={max}
    tabindex="0"
  >
    <!-- Track ring -->
    <svg viewBox="0 0 60 60" class="ring">
      <circle cx="30" cy="30" r="24" fill="none" stroke="rgba(255,255,255,0.08)" stroke-width="3"/>
      <!-- Active arc approximated as rotated line markers -->
    </svg>

    <!-- Knob body -->
    <div class="body" style="transform: rotate({angle}deg)">
      <div class="dot"></div>
    </div>

    <!-- Glow -->
    <div class="glow" style="opacity: {0.3 + value * 0.7}"></div>
  </div>

  <div class="value">{displayVal}{unit}</div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=Jost:wght@200;300;400&display=swap');

  .knob-wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 6px;
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

  .knob {
    position: relative;
    width: 60px;
    height: 60px;
    cursor: ns-resize;
  }

  .ring {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
  }

  .body {
    position: absolute;
    inset: 8px;
    border-radius: 50%;
    background: radial-gradient(circle at 35% 30%, rgba(255,255,255,0.18), rgba(180,170,220,0.06));
    backdrop-filter: blur(8px);
    border: 1px solid rgba(255,255,255,0.2);
    box-shadow:
      0 4px 16px rgba(0,0,0,0.35),
      inset 0 1px 0 rgba(255,255,255,0.25);
    transition: box-shadow 0.1s;
    display: flex;
    align-items: flex-start;
    justify-content: center;
    padding-top: 5px;
  }

  .knob.active .body {
    box-shadow:
      0 4px 20px rgba(0,0,0,0.4),
      0 0 14px rgba(255, 120, 60, 0.35),
      inset 0 1px 0 rgba(255,255,255,0.3);
  }

  .dot {
    width: 4px;
    height: 4px;
    border-radius: 50%;
    background: rgba(255, 130, 60, 0.95);
    box-shadow: 0 0 6px rgba(255,100,40,0.9);
  }

  .glow {
    position: absolute;
    inset: 8px;
    border-radius: 50%;
    background: radial-gradient(circle, rgba(255,110,40,0.2), transparent 70%);
    pointer-events: none;
    transition: opacity 0.15s;
  }

  .value {
    font-family: 'Jost', sans-serif;
    font-size: 0.6rem;
    font-weight: 200;
    color: rgba(255, 150, 80, 0.75);
    letter-spacing: 0.1em;
  }
</style>
