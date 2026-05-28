<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  const dispatch = createEventDispatcher<{ click: void }>();

  export let label: string  = '';
  export let on:    boolean = false;
</script>

<!-- svelte-ignore a11y-no-noninteractive-tabindex -->
<div
  class="lamp"
  class:on
  role="button"
  tabindex="0"
  on:click={() => dispatch('click')}
  on:keydown={e => e.key === 'Enter' && dispatch('click')}
>
  <div class="lamp-bulb"></div>
  <div class="lamp-label">{label}</div>
</div>

<style>
  .lamp {
    display: flex; flex-direction: column; align-items: center; gap: 8px;
    cursor: pointer; user-select: none;
  }
  .lamp-bulb {
    width: 28px; height: 28px; border-radius: 50%;
    background: var(--glass-bg-hi);
    border: 1px solid var(--glass-border);
    backdrop-filter: var(--glass-blur);
    -webkit-backdrop-filter: var(--glass-blur);
    display: flex; align-items: center; justify-content: center;
    transition: all 0.2s ease;
  }
  .lamp-bulb::after {
    content: ''; width: 7px; height: 7px; border-radius: 50%;
    background: rgba(180,170,200,0.20);
    transition: all 0.2s ease;
  }
  .lamp.on .lamp-bulb::after {
    background: var(--accent);
    box-shadow: 0 0 10px var(--accent-glow), 0 0 22px var(--accent-dim);
  }
  .lamp.on .lamp-bulb { border-color: var(--accent-line); }
  .lamp-label {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.56rem; font-weight: 400; letter-spacing: 0.30em;
    text-transform: uppercase; color: var(--text-lo);
  }
  .lamp.on .lamp-label { color: var(--accent); }
</style>
