<script>
  import { createEventDispatcher } from 'svelte';

  export let label  = 'ON';
  export let active = false;
  export let icon   = '';
  // 2b: teal glow + icon pulse when active && glow
  export let glow   = false;
  // 4d: small LED dot above the icon
  export let led    = false;
  // Pill shape variant for bypass button (2a)
  export let pill   = false;

  const dispatch = createEventDispatcher();

  function press() {
    active = !active;
    dispatch('change', { active });
  }
</script>

<div class="wrap">
  <button
    class="btn"
    class:active
    class:glow-active={glow && active}
    class:pill
    on:click={press}
    aria-pressed={active}
    aria-label={label}
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
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 7px;
    user-select: none;
  }

  .btn {
    width: 52px;
    height: 52px;
    border-radius: 50%;
    border: none;
    background: #e8e0d4;
    box-shadow:
      6px 6px 14px rgba(155,135,105,0.45),
      -5px -5px 12px rgba(255,252,244,0.92);
    cursor: pointer;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: 3px;
    transition: box-shadow 0.14s, background 0.14s;
    position: relative;
  }

  /* Pill variant */
  .btn.pill {
    width: auto;
    height: 36px;
    border-radius: 30px;
    padding: 0 16px;
    flex-direction: row;
    gap: 6px;
  }

  .btn:active:not(.active) {
    background: #e5ddd1;
  }

  .btn.active {
    background: #e2d9cc;
    box-shadow:
      inset 4px 4px 9px rgba(150,130,100,0.35),
      inset -3px -3px 7px rgba(255,250,240,0.75);
    transition: none;
  }

  /* 2b: teal glow when active + glow prop */
  .btn.glow-active {
    box-shadow:
      inset 4px 4px 9px rgba(150,130,100,0.35),
      inset -3px -3px 7px rgba(255,250,240,0.75),
      0 0 12px rgba(0, 200, 180, 0.6);
  }

  .icon {
    font-size: 1rem;
    color: rgba(130,105,70,0.55);
    line-height: 1;
    transition: color 0.14s;
  }

  .btn.active .icon {
    color: rgba(130,90,40,0.75);
  }

  /* 2b: pulse animation on icon when glow && active */
  @keyframes freeze-pulse {
    0%, 100% { opacity: 1; transform: scale(1); }
    50%       { opacity: 0.65; transform: scale(1.15); }
  }

  .icon.pulse {
    animation: freeze-pulse 2s ease-in-out infinite;
    color: rgba(0, 200, 180, 0.85) !important;
  }

  /* 4d: LED indicator */
  .led {
    width: 4px;
    height: 4px;
    border-radius: 50%;
    background: rgba(130, 110, 85, 0.2);
    transition: background 0.2s, box-shadow 0.2s;
    flex-shrink: 0;
  }

  .led.led-on {
    background: #00c8b4;
    box-shadow: 0 0 4px rgba(0, 200, 180, 0.7);
  }

  .pill-label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.6rem;
    font-weight: 300;
    letter-spacing: 0.14em;
    text-transform: uppercase;
    color: rgba(130,105,70,0.7);
  }

  .btn.active .pill-label {
    color: rgba(110,80,40,0.85);
  }

  .btn.glow-active .pill-label {
    color: rgba(0, 200, 180, 0.85);
  }

  .label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.7);
  }
</style>
