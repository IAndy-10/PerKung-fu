<script lang="ts">
  import { createEventDispatcher } from 'svelte';

  export let name: string;
  export let value: number;   // 0..1 normalized
  export let display: string;
  export let unit: string = '';

  const dispatch = createEventDispatcher<{ change: number }>();

  let trackEl: HTMLDivElement;

  function clamp(v: number) { return Math.max(0, Math.min(1, v)); }

  function onMouseDown(e: MouseEvent) {
    e.preventDefault();
    const update = (ev: MouseEvent) => {
      const r = trackEl.getBoundingClientRect();
      dispatch('change', clamp((ev.clientX - r.left) / r.width));
    };
    update(e);
    document.body.style.cursor = 'ew-resize';
    const onMove = (ev: MouseEvent) => update(ev);
    const onUp = () => {
      document.body.style.cursor = '';
      window.removeEventListener('mousemove', onMove);
      window.removeEventListener('mouseup', onUp);
    };
    window.addEventListener('mousemove', onMove);
    window.addEventListener('mouseup', onUp);
  }

  function onWheel(e: WheelEvent) {
    e.preventDefault();
    const step = e.shiftKey ? 0.05 : 0.01;
    dispatch('change', clamp(value + (e.deltaY < 0 ? step : -step)));
  }
</script>

<div class="prow">
  <div class="phead">
    <span class="pname">{name}</span>
    <span class="pval">
      <span class="num">{display}</span>
      <span class="punit">{unit || '──'}</span>
    </span>
  </div>
  <!-- svelte-ignore a11y-no-noninteractive-element-interactions -->
  <div
    bind:this={trackEl}
    class="ptrack"
    role="slider"
    tabindex="0"
    aria-valuenow={value}
    aria-valuemin={0}
    aria-valuemax={1}
    on:mousedown={onMouseDown}
    on:wheel|preventDefault={onWheel}
  >
    <div class="pfill" style="width:{value * 100}%"></div>
    <div class="pticks"></div>
    <div class="pcaret" style="left:{value * 100}%"></div>
  </div>
</div>
