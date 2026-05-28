<script>
  export let label = 'MODE';
  export let options = ['A', 'B', 'C'];
  export let selected = 0;
</script>

<div class="selector-wrap">
  <div class="label">{label}</div>
  <div class="options">
    {#each options as opt, i}
      <button
        class="opt"
        class:active={selected === i}
        on:click={() => selected = i}
      >
        <span class="pip" class:lit={selected === i}></span>
        <span class="text">{opt}</span>
      </button>
    {/each}
  </div>
</div>

<style>
  @import url('https://fonts.googleapis.com/css2?family=Jost:wght@200;300;400&display=swap');

  .selector-wrap {
    display: flex;
    flex-direction: column;
    gap: 8px;
    align-items: flex-start;
  }

  .label {
    font-family: 'Jost', sans-serif;
    font-size: 0.6rem;
    font-weight: 300;
    letter-spacing: 0.25em;
    text-transform: uppercase;
    color: rgba(180, 170, 210, 0.7);
    padding-left: 2px;
  }

  .options {
    display: flex;
    flex-direction: column;
    gap: 5px;
  }

  .opt {
    display: flex;
    align-items: center;
    gap: 8px;
    background: none;
    border: none;
    cursor: pointer;
    padding: 4px 10px 4px 6px;
    border-radius: 6px;
    background: rgba(255,255,255,0.04);
    border: 1px solid rgba(255,255,255,0.07);
    transition: background 0.15s, border-color 0.15s;
  }

  .opt.active {
    background: rgba(255,255,255,0.08);
    border-color: rgba(255,160,80,0.3);
  }

  .pip {
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: rgba(180,160,220,0.3);
    border: 1px solid rgba(255,255,255,0.2);
    flex-shrink: 0;
    transition: background 0.2s, box-shadow 0.2s;
  }

  .pip.lit {
    background: rgb(255, 110, 40);
    border-color: rgba(255,150,80,0.6);
    box-shadow: 0 0 6px rgba(255,90,30,0.8);
  }

  .text {
    font-family: 'Jost', sans-serif;
    font-size: 0.65rem;
    font-weight: 300;
    letter-spacing: 0.15em;
    text-transform: uppercase;
    color: rgba(210, 200, 240, 0.6);
    transition: color 0.15s;
  }

  .opt.active .text {
    color: rgba(255, 200, 160, 0.9);
  }
</style>
