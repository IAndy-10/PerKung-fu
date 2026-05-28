<script lang="ts">
  import { createEventDispatcher } from 'svelte';

  export let label  = 'ON';
  export let active = false;
  export let icon   = '';
  export let glow   = false;
  export let led    = false;
  export let pill   = false;

  const dispatch = createEventDispatcher();

  function press() {
    active = !active;
    dispatch('change', { active });
  }
</script>

<div class="wrap">
  <button
    class="btn" class:active class:glow-active={glow && active} class:pill
    on:click={press} aria-pressed={active} aria-label={label}
  >
    {#if led}
      <span class="led" class:led-on={active}></span>
    {/if}
    {#if icon}
      <span class="icon" class:pulse={glow && active}>{icon}</span>
    {/if}
    {#if pill}
      <span class="pill-label">{label}</span>
    {/if}
  </button>
  {#if !pill}
    <div class="label">{label}</div>
  {/if}
</div>

<style>
  .wrap {
    display: flex; flex-direction: column; align-items: center;
    gap: 7px; user-select: none;
  }

  .btn {
    width: 44px; height: 44px; border-radius: 50%;
    border: 1px solid rgba(255,255,255,0.12);
    background: rgba(255,255,255,0.06);
    backdrop-filter: blur(8px);
    box-shadow: 0 3px 12px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.12);
    cursor: pointer;
    display: flex; flex-direction: column; align-items: center;
    justify-content: center; gap: 3px;
    transition: box-shadow 0.2s, background 0.2s, border-color 0.2s;
    position: relative;
  }

  .btn.pill {
    width: auto; height: 34px; border-radius: 30px;
    padding: 0 16px; flex-direction: row; gap: 6px;
  }

  .btn.active {
    background: rgba(255,255,255,0.1);
    border-color: rgba(255,140,60,0.4);
    box-shadow: 0 3px 12px rgba(0,0,0,0.4), 0 0 20px rgba(255,100,40,0.3),
      inset 0 1px 0 rgba(255,255,255,0.2);
  }

  .btn.glow-active {
    box-shadow: 0 3px 12px rgba(0,0,0,0.4), 0 0 28px rgba(255,100,40,0.55),
      inset 0 1px 0 rgba(255,255,255,0.2);
  }

  .led {
    width: 5px; height: 5px; border-radius: 50%;
    background: rgba(180,100,60,0.25);
    border: 1px solid rgba(255,255,255,0.12);
    transition: background 0.2s, box-shadow 0.2s; flex-shrink: 0;
  }
  .led.led-on {
    background: rgb(255,110,40);
    border-color: rgba(255,150,80,0.6);
    box-shadow: 0 0 6px rgba(255,90,30,0.9);
  }

  .icon {
    font-size: 0.9rem; color: rgba(180,170,210,0.45);
    line-height: 1; transition: color 0.14s;
  }
  .btn.active .icon { color: rgba(255,160,100,0.85); }

  @keyframes freeze-pulse {
    0%, 100% { opacity: 1; transform: scale(1); }
    50%       { opacity: 0.65; transform: scale(1.15); }
  }
  .icon.pulse { animation: freeze-pulse 2s ease-in-out infinite; color: rgba(255,130,60,0.9) !important; }

  .pill-label {
    font-family: 'Jost', sans-serif; font-size: 0.56rem; font-weight: 300;
    letter-spacing: 0.14em; text-transform: uppercase;
    color: rgba(180,170,210,0.6); transition: color 0.14s;
  }
  .btn.active .pill-label { color: rgba(255,160,100,0.92); }

  .label {
    font-family: 'Jost', sans-serif; font-size: 0.54rem; font-weight: 300;
    letter-spacing: 0.18em; text-transform: uppercase; color: rgba(180,170,210,0.52);
  }
</style>
