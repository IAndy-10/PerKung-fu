<script>
  import { createEventDispatcher, onMount, onDestroy } from 'svelte';

  export let label    = 'MODE';
  export let options  = ['A', 'B', 'C'];
  export let selected = 0;

  const dispatch = createEventDispatcher();

  let open = false;
  let triggerEl;

  function toggle() {
    open = !open;
  }

  function choose(i) {
    selected = i;
    open = false;
    dispatch('change', { selected });
  }

  function onOutsideClick(e) {
    if (triggerEl && !triggerEl.contains(e.target)) {
      open = false;
    }
  }

  onMount(()  => window.addEventListener('mousedown', onOutsideClick));
  onDestroy(() => window.removeEventListener('mousedown', onOutsideClick));
</script>

<div class="wrap" bind:this={triggerEl}>
  <div class="label">{label}</div>

  <!-- Trigger button -->
  <button
    class="trigger"
    class:open
    on:click={toggle}
    aria-haspopup="listbox"
    aria-expanded={open}
  >
    <span class="trigger-text">{options[selected] ?? '—'}</span>
    <span class="chevron" class:flipped={open}>▾</span>
  </button>

  <!-- Dropdown -->
  {#if open}
    <div class="dropdown" role="listbox">
      {#each options as opt, i}
        <button
          class="option"
          class:active={i === selected}
          role="option"
          aria-selected={i === selected}
          on:click={() => choose(i)}
        >
          {opt}
        </button>
      {/each}
    </div>
  {/if}
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=DM+Sans:wght@200;300;400&display=swap');

  .wrap {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 7px;
    user-select: none;
    position: relative;
  }

  .label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.7);
  }

  /* ── Trigger ─────────────────────────────────────────── */
  .trigger {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
    min-width: 80px;
    padding: 7px 12px;
    border-radius: 10px;
    border: none;
    background: #e8e0d4;
    box-shadow:
      6px 6px 14px rgba(155,135,105,0.45),
      -5px -5px 12px rgba(255,252,244,0.92);
    cursor: pointer;
    transition: box-shadow 0.14s, background 0.14s;
  }

  .trigger:active:not(.open) {
    background: #e5ddd1;
  }

  .trigger.open {
    background: #e2d9cc;
    box-shadow:
      inset 4px 4px 9px rgba(150,130,100,0.35),
      inset -3px -3px 7px rgba(255,250,240,0.75);
    transition: none;
  }

  .trigger-text {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.68rem;
    font-weight: 300;
    letter-spacing: 0.1em;
    text-transform: uppercase;
    color: rgba(120, 95, 65, 0.85);
  }

  .chevron {
    font-size: 0.7rem;
    color: rgba(130,105,70,0.5);
    display: inline-block;
    transition: transform 0.14s;
    line-height: 1;
  }

  .chevron.flipped {
    transform: rotate(180deg);
  }

  /* ── Dropdown panel ──────────────────────────────────── */
  .dropdown {
    position: absolute;
    top: calc(100% + 6px);
    left: 50%;
    transform: translateX(-50%);
    z-index: 100;
    display: flex;
    flex-direction: column;
    gap: 4px;
    padding: 6px;
    border-radius: 12px;
    background: #ede6da;
    box-shadow:
      8px 8px 20px rgba(155,135,105,0.38),
      -6px -6px 16px rgba(255,252,244,0.92);
    min-width: 100%;
  }

  /* ── Option items ─────────────────────────────────────── */
  .option {
    width: 100%;
    padding: 6px 12px;
    border-radius: 8px;
    border: none;
    background: transparent;
    cursor: pointer;
    font-family: 'DM Sans', sans-serif;
    font-size: 0.65rem;
    font-weight: 300;
    letter-spacing: 0.1em;
    text-transform: uppercase;
    color: rgba(120, 95, 65, 0.7);
    text-align: left;
    transition: box-shadow 0.12s, background 0.12s;
  }

  .option:hover:not(.active) {
    background: #e8e0d4;
    box-shadow:
      3px 3px 7px rgba(155,135,105,0.3),
      -2px -2px 5px rgba(255,252,244,0.8);
    color: rgba(110, 85, 55, 0.85);
  }

  .option.active {
    background: #e2d9cc;
    box-shadow:
      inset 3px 3px 7px rgba(150,130,100,0.3),
      inset -2px -2px 5px rgba(255,250,240,0.7);
    color: rgba(110, 82, 45, 0.9);
  }
</style>
