<script lang="ts">
  import { createEventDispatcher, onMount, onDestroy } from 'svelte';

  export let label    = 'MODE';
  export let options  = ['A', 'B', 'C'];
  export let selected = 0;

  const dispatch = createEventDispatcher();

  let open = false;
  let triggerEl;

  function toggle() { open = !open; }

  function choose(i) {
    selected = i; open = false;
    dispatch('change', { selected });
  }

  function onOutsideClick(e) {
    if (triggerEl && !triggerEl.contains(e.target)) open = false;
  }

  onMount(()  => window.addEventListener('mousedown', onOutsideClick));
  onDestroy(() => window.removeEventListener('mousedown', onOutsideClick));
</script>

<div class="wrap" bind:this={triggerEl}>
  <div class="label">{label}</div>

  <button class="trigger" class:open on:click={toggle}
    aria-haspopup="listbox" aria-expanded={open}>
    <span class="trigger-text">{options[selected] ?? '—'}</span>
    <span class="chevron" class:flipped={open}>▾</span>
  </button>

  {#if open}
    <div class="dropdown" role="listbox">
      {#each options as opt, i}
        <button
          class="option" class:active={i === selected}
          role="option" aria-selected={i === selected}
          on:click={() => choose(i)}
        >
          <span class="pip" class:lit={i === selected}></span>
          {opt}
        </button>
      {/each}
    </div>
  {/if}
</div>

<style>
  .wrap {
    display: flex; flex-direction: column; align-items: flex-start;
    gap: 7px; user-select: none; position: relative;
  }

  .label {
    font-family: 'Jost', sans-serif; font-size: 0.54rem; font-weight: 300;
    letter-spacing: 0.2em; text-transform: uppercase; color: rgba(180,170,210,0.52);
  }

  .trigger {
    display: flex; align-items: center; justify-content: space-between;
    gap: 8px; min-width: 100px; padding: 7px 12px; border-radius: 8px;
    border: 1px solid rgba(255,255,255,0.12);
    background: rgba(255,255,255,0.06);
    backdrop-filter: blur(8px);
    box-shadow: 0 2px 8px rgba(0,0,0,0.35);
    cursor: pointer; transition: border-color 0.15s, background 0.15s;
  }
  .trigger.open {
    border-color: rgba(255,140,60,0.4);
    background: rgba(255,255,255,0.09);
  }

  .trigger-text {
    font-family: 'Jost', sans-serif; font-size: 0.62rem; font-weight: 300;
    letter-spacing: 0.1em; text-transform: uppercase;
    color: rgba(210,200,240,0.75);
  }

  .chevron {
    font-size: 0.62rem; color: rgba(180,170,210,0.4);
    display: inline-block; transition: transform 0.14s; line-height: 1;
  }
  .chevron.flipped { transform: rotate(180deg); }

  .dropdown {
    position: absolute; top: calc(100% + 6px); left: 0; z-index: 200;
    display: flex; flex-direction: column; gap: 3px; padding: 6px;
    border-radius: 10px;
    background: rgba(12,8,20,0.97);
    border: 1px solid rgba(255,255,255,0.1);
    backdrop-filter: blur(16px);
    box-shadow: 0 8px 32px rgba(0,0,0,0.55);
    min-width: 100%;
  }

  .option {
    display: flex; align-items: center; gap: 8px;
    width: 100%; padding: 5px 10px; border-radius: 6px;
    border: 1px solid transparent; background: transparent;
    cursor: pointer;
    font-family: 'Jost', sans-serif; font-size: 0.6rem; font-weight: 300;
    letter-spacing: 0.12em; text-transform: uppercase;
    color: rgba(200,190,230,0.6); text-align: left;
    transition: background 0.12s, border-color 0.12s, color 0.12s;
  }
  .option:hover:not(.active) { background: rgba(255,255,255,0.06); color: rgba(210,200,240,0.8); }
  .option.active {
    background: rgba(255,110,40,0.15); border-color: rgba(255,140,60,0.28);
    color: rgba(255,170,100,0.92);
  }

  .pip {
    width: 5px; height: 5px; border-radius: 50%;
    background: rgba(180,160,220,0.2);
    border: 1px solid rgba(255,255,255,0.14); flex-shrink: 0;
    transition: background 0.2s, box-shadow 0.2s;
  }
  .pip.lit {
    background: rgb(255,110,40); border-color: rgba(255,150,80,0.6);
    box-shadow: 0 0 5px rgba(255,90,30,0.85);
  }
</style>
