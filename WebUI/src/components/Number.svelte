<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  import { tooltipStore } from '../stores/tooltip';

  export let label    = 'VALUE';
  export let value    = 0;
  export let min      = 0;
  export let max      = 100;
  export let step     = 1;
  export let unit     = '';
  export let decimals = 1;

  const dispatch = createEventDispatcher();

  let editing  = false;
  let inputEl: HTMLInputElement | undefined;
  let rawInput = '';

  $: displayVal = (unit === ' ms' && value >= 1000)
    ? value.toFixed(0)
    : value.toFixed(decimals);

  function clamp(v: number) { return Math.min(max, Math.max(min, v)); }
  function emit() { dispatch('change', { value }); }

  function startEdit() {
    if (editing) return;  // already in edit mode — ignore re-clicks
    rawInput = displayVal;
    editing  = true;
    // Use setTimeout so the <input> renders before we focus it.
    // We do NOT set rawInput to the key that was pressed here, so no character
    // can "fall through" from the triggering event into the input.
    setTimeout(() => { if (inputEl) { inputEl.focus(); inputEl.select(); } }, 0);
  }

  function commitEdit() {
    const p = parseFloat(rawInput);
    if (!isNaN(p)) { value = clamp(p); emit(); }
    editing = false;
  }

  function cancelEdit() { editing = false; }

  function onInputKey(e: KeyboardEvent) {
    if (e.key === 'Enter')       { e.preventDefault(); commitEdit(); }
    else if (e.key === 'Escape') { e.preventDefault(); cancelEdit(); }
    else if (e.key === 'ArrowUp') {
      e.preventDefault();
      rawInput = clamp(parseFloat(rawInput || String(value)) + step).toFixed(decimals);
    }
    else if (e.key === 'ArrowDown') {
      e.preventDefault();
      rawInput = clamp(parseFloat(rawInput || String(value)) - step).toFixed(decimals);
    }
  }

  function onDisplayKey(e: KeyboardEvent) {
    if (e.key === 'ArrowUp') {
      e.preventDefault(); value = clamp(value + step); emit();
    } else if (e.key === 'ArrowDown') {
      e.preventDefault(); value = clamp(value - step); emit();
    } else if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault(); startEdit();
    }
    // Digit keys are intentionally NOT handled here.
    // A single click already opens the input with the current value selected,
    // so the user can just type to overwrite — no shortcut needed.
  }

  function onWheel(e: WheelEvent) {
    // preventDefault is called via the |preventDefault modifier on the element.
    value = clamp(value + (e.deltaY < 0 ? 1 : -1) * step);
    emit();
  }

  let ttTimer: ReturnType<typeof setTimeout>;
  function onMouseEnter(e: MouseEvent) {
    ttTimer = setTimeout(() => tooltipStore.set({
      visible: true, x: e.clientX + 12, y: e.clientY - 48,
      name: label, value: `${displayVal}${unit}`, range: `${min} – ${max}${unit}`,
    }), 600);
  }
  function onMouseLeave() {
    clearTimeout(ttTimer);
    tooltipStore.update(s => ({ ...s, visible: false }));
  }
  function onMouseMove(e: MouseEvent) {
    tooltipStore.update(s => s.visible ? { ...s, x: e.clientX + 12, y: e.clientY - 48 } : s);
  }

  let ctxVisible = false, ctxX = 0, ctxY = 0;
  function onContextMenu(e: MouseEvent) {
    e.preventDefault(); ctxX = e.clientX; ctxY = e.clientY; ctxVisible = true;
    setTimeout(() => window.addEventListener('mousedown', closeCtx, { once: true }), 0);
  }
  function closeCtx() { ctxVisible = false; }
  function ctxSetDefault() { value = clamp((min + max) / 2); emit(); closeCtx(); }
  function ctxEnterValue()  { startEdit(); closeCtx(); }
  async function ctxCopy()  { await navigator.clipboard.writeText(displayVal).catch(() => {}); closeCtx(); }
  async function ctxPaste() {
    try {
      const v = parseFloat(await navigator.clipboard.readText());
      if (!isNaN(v)) { value = clamp(v); emit(); }
    } catch {} closeCtx();
  }
</script>

<!-- svelte-ignore a11y-no-noninteractive-tabindex -->
<div class="wrap"
  on:mouseenter={onMouseEnter} on:mouseleave={onMouseLeave}
  on:mousemove={onMouseMove}   on:contextmenu={onContextMenu}
>
  <div class="label">{label}</div>

  <div class="display" class:editing tabindex="0"
    role="spinbutton"
    aria-label="{label} {displayVal}{unit}"
    aria-valuenow={value} aria-valuemin={min} aria-valuemax={max}
    on:click={startEdit}
    on:keydown={onDisplayKey}
    on:wheel|preventDefault={onWheel}
  >
    {#if editing}
      <input bind:this={inputEl} bind:value={rawInput}
        class="raw-input" type="text" inputmode="decimal"
        aria-label="Enter value for {label}"
        on:keydown={onInputKey} on:blur={commitEdit} />
    {:else}
      <span class="val-text">{displayVal}{unit}</span>
    {/if}
  </div>

  {#if ctxVisible}
    <div class="ctx-menu" style="left:{ctxX}px;top:{ctxY}px;" role="menu">
      <button class="ctx-item" on:click={ctxSetDefault}>Set to default</button>
      <button class="ctx-item" on:click={ctxEnterValue}>Enter value</button>
      <button class="ctx-item" on:click={ctxCopy}>Copy value</button>
      <button class="ctx-item" on:click={ctxPaste}>Paste value</button>
    </div>
  {/if}
</div>

<style>
  .wrap {
    display: flex; flex-direction: column; align-items: center;
    gap: 5px; user-select: none; position: relative;
  }

  .label {
    font-family: 'Jost', sans-serif; font-size: 0.52rem; font-weight: 300;
    letter-spacing: 0.2em; text-transform: uppercase; color: rgba(180,170,210,0.52);
  }

  .display {
    min-width: 72px; height: 28px; padding: 0 8px; border-radius: 7px;
    background: rgba(255,255,255,0.05);
    border: 1px solid rgba(255,255,255,0.1);
    backdrop-filter: blur(6px);
    box-shadow: inset 0 2px 4px rgba(0,0,0,0.35);
    display: flex; align-items: center; justify-content: center;
    cursor: text; outline: none; transition: border-color 0.12s;
  }
  .display:focus { border-color: rgba(255,140,60,0.35); box-shadow: inset 0 2px 4px rgba(0,0,0,0.35), 0 0 0 1.5px rgba(255,110,40,0.2); }
  .display.editing { border-color: rgba(255,140,60,0.45); }

  .val-text {
    font-family: 'Jost', sans-serif; font-size: 0.64rem; font-weight: 200;
    color: rgba(255,150,80,0.82); letter-spacing: 0.05em; pointer-events: none;
  }
  .raw-input {
    width: 100%; background: transparent; border: none; outline: none;
    font-family: 'Jost', sans-serif; font-size: 0.64rem; font-weight: 200;
    color: rgba(255,170,100,0.95); letter-spacing: 0.05em;
    text-align: center; padding: 0;
  }

  .ctx-menu {
    position: fixed; z-index: 9998; background: rgba(12,8,20,0.97);
    border: 1px solid rgba(255,255,255,0.1); border-radius: 10px; padding: 6px;
    display: flex; flex-direction: column; gap: 2px;
    box-shadow: 0 8px 32px rgba(0,0,0,0.55); min-width: 120px; backdrop-filter: blur(12px);
  }
  .ctx-item {
    font-family: 'Jost', sans-serif; font-size: 0.6rem; font-weight: 300;
    letter-spacing: 0.08em; color: rgba(200,190,230,0.7);
    background: transparent; border: none; border-radius: 6px;
    padding: 6px 10px; text-align: left; cursor: pointer; transition: background 0.12s;
  }
  .ctx-item:hover { background: rgba(255,110,40,0.15); color: rgba(255,170,100,0.9); }
</style>
