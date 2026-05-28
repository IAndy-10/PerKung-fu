<script>
  export let label = 'PWR';
  export let active = false;

  function toggle() { active = !active; }
</script>

<div class="toggle-wrap">
  <button
    class="btn"
    class:active
    on:click={toggle}
    aria-pressed={active}
  >
    <div class="led" class:on={active}></div>
    <div class="led-glow" class:on={active}></div>
  </button>
  <div class="label">{label}</div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=Jost:wght@200;300&display=swap');

  .toggle-wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 7px;
    user-select: none;
  }

  .btn {
    position: relative;
    width: 36px;
    height: 36px;
    border-radius: 50%;
    background: rgba(255,255,255,0.06);
    border: 1px solid rgba(255,255,255,0.15);
    backdrop-filter: blur(8px);
    box-shadow: 0 3px 12px rgba(0,0,0,0.35), inset 0 1px 0 rgba(255,255,255,0.15);
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    transition: box-shadow 0.2s, background 0.2s;
  }

  .btn.active {
    background: rgba(255,255,255,0.1);
    box-shadow:
      0 3px 12px rgba(0,0,0,0.35),
      0 0 20px rgba(255,100,40,0.45),
      inset 0 1px 0 rgba(255,255,255,0.2);
  }

  .led {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: rgba(180,100,60,0.4);
    transition: background 0.2s;
    z-index: 1;
  }

  .led.on {
    background: rgb(255, 110, 40);
    box-shadow: 0 0 6px rgba(255,100,30,0.9);
  }

  .led-glow {
    position: absolute;
    inset: 4px;
    border-radius: 50%;
    background: radial-gradient(circle, rgba(255,100,40,0.3), transparent 70%);
    opacity: 0;
    transition: opacity 0.2s;
    pointer-events: none;
  }

  .led-glow.on { opacity: 1; }

  .label {
    font-family: 'Jost', sans-serif;
    font-size: 0.6rem;
    font-weight: 300;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: rgba(180, 170, 210, 0.7);
  }
</style>
