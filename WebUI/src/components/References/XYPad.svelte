<script>
  export let labelX = 'CUTOFF';
  export let labelY = 'RESO';
  export let x = 0.5;
  export let y = 0.5;

  let padEl;
  let dragging = false;

  function getPos(e) {
    const rect = padEl.getBoundingClientRect();
    return {
      x: Math.min(1, Math.max(0, (e.clientX - rect.left) / rect.width)),
      y: Math.min(1, Math.max(0, 1 - (e.clientY - rect.top) / rect.height)),
    };
  }

  function onMouseDown(e) {
    dragging = true;
    const p = getPos(e);
    x = p.x; y = p.y;
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup', onMouseUp);
  }

  function onMouseMove(e) {
    if (!dragging) return;
    const p = getPos(e);
    x = p.x; y = p.y;
  }

  function onMouseUp() {
    dragging = false;
    window.removeEventListener('mousemove', onMouseMove);
    window.removeEventListener('mouseup', onMouseUp);
  }
</script>

<div class="xy-wrap">
  <div class="top-labels">
    <span class="lbl">{labelX}</span>
    <span class="lbl">{labelY}</span>
  </div>

  <div
    class="pad"
    bind:this={padEl}
    on:mousedown={onMouseDown}
    class:dragging
    role="slider"
    tabindex="0"
  >
    <!-- Grid lines -->
    <div class="grid-h" style="bottom: {y * 100}%"></div>
    <div class="grid-v" style="left: {x * 100}%"></div>

    <!-- Crosshair dot -->
    <div
      class="dot"
      style="left: {x * 100}%; bottom: {y * 100}%"
    >
      <div class="pulse"></div>
    </div>

    <!-- Corner labels -->
    <span class="corner tl">↖</span>
    <span class="corner tr">↗</span>
    <span class="corner bl">↙</span>
    <span class="corner br">↘</span>
  </div>

  <div class="vals">
    <span>{(x * 100).toFixed(0)}</span>
    <span>{(y * 100).toFixed(0)}</span>
  </div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=Jost:wght@200;300&display=swap');

  .xy-wrap {
    display: flex;
    flex-direction: column;
    gap: 7px;
    user-select: none;
  }

  .top-labels {
    display: flex;
    justify-content: space-between;
  }

  .lbl {
    font-family: 'Jost', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: rgba(180, 170, 210, 0.7);
  }

  .pad {
    position: relative;
    width: 120px;
    height: 120px;
    background: rgba(255,255,255,0.04);
    border: 1px solid rgba(255,255,255,0.14);
    border-radius: 10px;
    cursor: crosshair;
    overflow: hidden;
    backdrop-filter: blur(6px);
    box-shadow: inset 0 1px 0 rgba(255,255,255,0.1), 0 4px 16px rgba(0,0,0,0.3);
  }

  .pad.dragging {
    border-color: rgba(255,140,60,0.4);
    box-shadow: inset 0 1px 0 rgba(255,255,255,0.1), 0 4px 20px rgba(255,100,40,0.2);
  }

  .grid-h, .grid-v {
    position: absolute;
    background: rgba(255,255,255,0.07);
    pointer-events: none;
  }

  .grid-h {
    left: 0; right: 0;
    height: 1px;
  }

  .grid-v {
    top: 0; bottom: 0;
    width: 1px;
  }

  .dot {
    position: absolute;
    width: 12px;
    height: 12px;
    transform: translate(-50%, 50%);
    pointer-events: none;
    z-index: 2;
  }

  .dot::after {
    content: '';
    position: absolute;
    inset: 2px;
    border-radius: 50%;
    background: rgb(255, 110, 40);
    box-shadow: 0 0 8px rgba(255,90,30,0.9);
  }

  .pulse {
    position: absolute;
    inset: 0;
    border-radius: 50%;
    border: 1px solid rgba(255,110,40,0.5);
    animation: pulse 2s ease-in-out infinite;
  }

  @keyframes pulse {
    0%, 100% { transform: scale(1); opacity: 0.5; }
    50% { transform: scale(2.2); opacity: 0; }
  }

  .corner {
    position: absolute;
    font-size: 0.45rem;
    color: rgba(255,255,255,0.15);
    line-height: 1;
  }
  .tl { top: 4px; left: 5px; }
  .tr { top: 4px; right: 5px; }
  .bl { bottom: 4px; left: 5px; }
  .br { bottom: 4px; right: 5px; }

  .vals {
    display: flex;
    justify-content: space-between;
    font-family: 'Jost', sans-serif;
    font-size: 0.58rem;
    font-weight: 200;
    color: rgba(255,150,80,0.7);
    letter-spacing: 0.1em;
  }
</style>
