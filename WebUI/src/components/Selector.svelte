<script lang="ts">
  import { createEventDispatcher } from 'svelte';

  export let label    = 'MODE';
  export let options  = ['A', 'B', 'C'];
  export let selected = 0;

  const dispatch = createEventDispatcher();

  function select(i) {
    dispatch('change', { selected: i });
  }
</script>

<div class="wrap">
  {#if label}
    <div class="label">{label}</div>
  {/if}
  <div class="track">
    {#each options as opt, i}
      <button
        class="opt" class:active={selected === i}
        aria-pressed={selected === i}
        on:click={() => select(i)}
      >{opt}</button>
    {/each}
  </div>
</div>

<style>
  .wrap {
    display: flex; flex-direction: column;
    gap: 7px; align-items: center; user-select: none;
  }

  .label {
    font-family: 'Jost', sans-serif; font-size: 0.54rem; font-weight: 300;
    letter-spacing: 0.2em; text-transform: uppercase; color: rgba(180,170,210,0.52);
  }

  .track {
    display: flex;
    background: rgba(255,255,255,0.04);
    border: 1px solid rgba(255,255,255,0.1);
    border-radius: 8px; padding: 3px; gap: 2px;
  }

  .opt {
    font-family: 'Jost', sans-serif; font-size: 0.56rem; font-weight: 300;
    letter-spacing: 0.1em; text-transform: uppercase;
    color: rgba(180,170,210,0.45);
    border: 1px solid transparent; background: transparent;
    padding: 4px 10px; border-radius: 5px; cursor: pointer;
    transition: all 0.15s;
  }

  .opt.active {
    background: rgba(255,110,40,0.18);
    border-color: rgba(255,140,60,0.3);
    color: rgba(255,170,100,0.92);
    box-shadow: 0 0 8px rgba(255,90,30,0.18);
  }
</style>
