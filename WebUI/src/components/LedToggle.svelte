<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  const dispatch = createEventDispatcher<{ change: boolean }>();

  export let label: string  = '';
  export let active: boolean = false;

  function handleClick() { dispatch('change', !active); }
</script>

<!-- svelte-ignore a11y-no-noninteractive-tabindex -->
<div
  class="led-toggle"
  class:active
  role="button"
  tabindex="0"
  on:click={handleClick}
  on:keydown={e => e.key === 'Enter' && handleClick()}
>
  <div class="led-dot" />
  <span>{label}</span>
</div>

<style>
  .led-toggle {
    display: flex; align-items: center; gap: 9px;
    padding: 7px 13px 7px 10px;
    background: var(--glass-bg);
    border: 1px solid var(--glass-border);
    border-radius: 9px;
    cursor: pointer; user-select: none;
    transition: all 0.15s;
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.56rem; font-weight: 400; letter-spacing: 0.22em;
    text-transform: uppercase; color: var(--text-lo);
    white-space: nowrap;
    backdrop-filter: blur(8px);
    -webkit-backdrop-filter: blur(8px);
  }
  .led-toggle:hover { border-color: var(--glass-border-hi); }
  .led-toggle.active {
    color: var(--text-hi);
    border-color: var(--accent-line);
    background: rgba(255,182,72,0.06);
  }
  .led-dot {
    width: 7px; height: 7px; border-radius: 50%;
    background: rgba(180,170,200,0.20); flex-shrink: 0;
  }
  .led-toggle.active .led-dot {
    background: var(--accent);
    box-shadow: 0 0 8px var(--accent-glow), 0 0 14px var(--accent-dim);
  }
</style>
