<script>
  import { createEventDispatcher } from 'svelte';

  export let label = 'MODE';
  export let options = ['A', 'B', 'C'];
  export let selected = 0;

  const dispatch = createEventDispatcher();

  // 1c: parent owns state — do NOT mutate selected prop here
  function select(i) {
    dispatch('change', { selected: i });
  }
</script>

<div class="wrap">
  <div class="label">{label}</div>
  <div class="track">
    {#each options as opt, i}
      <button
        class="opt"
        class:active={selected === i}
        aria-pressed={selected === i}
        on:click={() => select(i)}
      >{opt}</button>
    {/each}
  </div>
</div>

<style>
  .wrap {
    display: flex;
    flex-direction: column;
    gap: 8px;
    align-items: center;
    user-select: none;
  }

  .label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.7);
  }

  .track {
    display: flex;
    border-radius: 30px;
    background: #e8e0d4;
    box-shadow:
      inset 3px 3px 7px rgba(150,130,100,0.3),
      inset -2px -2px 5px rgba(255,250,240,0.8);
    padding: 4px;
    gap: 3px;
  }

  .opt {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.6rem;
    font-weight: 300;
    letter-spacing: 0.12em;
    text-transform: uppercase;
    color: rgba(130,105,75,0.55);
    border: none;
    background: transparent;
    padding: 5px 12px;
    border-radius: 24px;
    cursor: pointer;
    transition: all 0.18s;
  }

  .opt.active {
    background: #ede6da;
    color: rgba(120,90,50,0.85);
    box-shadow:
      3px 3px 7px rgba(150,130,100,0.35),
      -2px -2px 5px rgba(255,252,242,0.85);
  }
</style>
