<script>
  import { tooltipStore } from '../stores/tooltip';
</script>

{#if $tooltipStore.visible}
  <div
    class="tooltip"
    style="left: {$tooltipStore.x}px; top: {$tooltipStore.y}px;"
    role="tooltip"
  >
    <div class="tt-name">{$tooltipStore.name}</div>
    <div class="tt-value">{$tooltipStore.value}</div>
    {#if $tooltipStore.range}
      <div class="tt-range">{$tooltipStore.range}</div>
    {/if}
  </div>
{/if}

<style>
  .tooltip {
    position: fixed;
    z-index: 9999;
    pointer-events: none;
    background: rgba(30, 25, 20, 0.88);
    border-left: 2px solid #00c8b4;
    border-radius: 8px;
    padding: 6px 10px;
    display: flex;
    flex-direction: column;
    gap: 2px;
    min-width: 100px;
    max-width: 180px;
    box-shadow: 4px 4px 12px rgba(0,0,0,0.4);
  }

  .tt-name {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.58rem;
    font-weight: 300;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(0, 200, 180, 0.85);
  }

  .tt-value {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.65rem;
    font-weight: 400;
    color: rgba(240, 230, 215, 0.92);
    letter-spacing: 0.06em;
  }

  .tt-range {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.52rem;
    font-weight: 200;
    color: rgba(200, 185, 165, 0.55);
    letter-spacing: 0.06em;
  }
</style>
