<script>
  import { createEventDispatcher } from 'svelte';
  import { tooltipStore } from '../stores/tooltip';

  export let label = 'VALUE';
  export let value = 0;
  export let min = 0;
  export let max = 100;
  export let step = 1;
  export let unit = '';
  export let decimals = 1;

  const dispatch = createEventDispatcher();

  let editing = false;
  let inputEl;
  let rawInput = '';

  // 1b: smart ms formatting — no decimals above 1000 ms
  $: displayVal = (unit === ' ms' && value >= 1000)
    ? value.toFixed(0)
    : value.toFixed(decimals);

  function clamp(v) {
    return Math.min(max, Math.max(min, v));
  }

  function emit() {
    dispatch('change', { value });
  }

  // ── Edit mode ──────────────────────────────────────────────
  function startEdit() {
    editing = true;
    rawInput = displayVal;
    setTimeout(() => {
      if (inputEl) { inputEl.focus(); inputEl.select(); }
    }, 0);
  }

  function commitEdit() {
    const parsed = parseFloat(rawInput);
    if (!isNaN(parsed)) {
      value = clamp(parsed);
      emit();
    }
    editing = false;
  }

  function cancelEdit() {
    editing = false;
  }

  function onInputKey(e) {
    if (e.key === 'Enter')      { e.preventDefault(); commitEdit(); }
    else if (e.key === 'Escape'){ e.preventDefault(); cancelEdit(); }
    else if (e.key === 'ArrowUp')   { e.preventDefault(); rawInput = clamp(parseFloat(rawInput || String(value)) + step).toFixed(decimals); }
    else if (e.key === 'ArrowDown') { e.preventDefault(); rawInput = clamp(parseFloat(rawInput || String(value)) - step).toFixed(decimals); }
  }

  // ── Display mode keyboard ──────────────────────────────────
  function onDisplayKey(e) {
    if (e.key === 'ArrowUp') {
      e.preventDefault(); value = clamp(value + step); emit();
    } else if (e.key === 'ArrowDown') {
      e.preventDefault(); value = clamp(value - step); emit();
    } else if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault(); startEdit();
    } else if ((e.key >= '0' && e.key <= '9') || e.key === '-' || e.key === '.') {
      rawInput = e.key;
      editing = true;
      setTimeout(() => {
        if (inputEl) { inputEl.focus(); inputEl.setSelectionRange(rawInput.length, rawInput.length); }
      }, 0);
    }
  }

  // ── Scroll ─────────────────────────────────────────────────
  function onWheel(e) {
    e.preventDefault();
    const dir = e.deltaY < 0 ? 1 : -1;
    value = clamp(value + dir * step);
    emit();
  }

  // ── Tooltip (4b) ───────────────────────────────────────────
  let ttTimer;
  function onMouseEnter(e) {
    ttTimer = setTimeout(() => {
      tooltipStore.set({
        visible: true,
        x: e.clientX + 12,
        y: e.clientY - 48,
        name: label,
        value: `${displayVal}${unit}`,
        range: `${min} – ${max}${unit}`,
      });
    }, 600);
  }
  function onMouseLeave() {
    clearTimeout(ttTimer);
    tooltipStore.update(s => ({ ...s, visible: false }));
  }
  function onMouseMove(e) {
    tooltipStore.update(s => s.visible
      ? { ...s, x: e.clientX + 12, y: e.clientY - 48 }
      : s
    );
  }

  // ── Context menu (2e) ──────────────────────────────────────
  let ctxVisible = false;
  let ctxX = 0, ctxY = 0;

  function onContextMenu(e) {
    e.preventDefault();
    ctxX = e.clientX; ctxY = e.clientY;
    ctxVisible = true;
    setTimeout(() => window.addEventListener('mousedown', closeCtx, { once: true }), 0);
  }
  function closeCtx() { ctxVisible = false; }

  function ctxSetDefault() {
    value = clamp((min + max) / 2); emit(); closeCtx();
  }
  function ctxEnterValue() { startEdit(); closeCtx(); }
  async function ctxCopy() {
    await navigator.clipboard.writeText(`${displayVal}`).catch(() => {});
    closeCtx();
  }
  async function ctxPaste() {
    try {
      const text = await navigator.clipboard.readText();
      const v = parseFloat(text);
      if (!isNaN(v)) { value = clamp(v); emit(); }
    } catch {}
    closeCtx();
  }
</script>

<!-- svelte-ignore a11y-no-noninteractive-tabindex -->
<div
  class="wrap"
  on:mouseenter={onMouseEnter}
  on:mouseleave={onMouseLeave}
  on:mousemove={onMouseMove}
  on:contextmenu={onContextMenu}
>
  <div class="label">{label}</div>

  <div
    class="display"
    class:editing
    tabindex="0"
    role="spinbutton"
    aria-label="{label} {displayVal}{unit}"
    aria-valuenow={value}
    aria-valuemin={min}
    aria-valuemax={max}
    on:dblclick={startEdit}
    on:keydown={onDisplayKey}
    on:wheel|passive={onWheel}
  >
    {#if editing}
      <input
        bind:this={inputEl}
        bind:value={rawInput}
        class="raw-input"
        type="text"
        inputmode="decimal"
        aria-label="Enter value for {label}"
        on:keydown={onInputKey}
        on:blur={commitEdit}
      />
    {:else}
      <span class="val-text">{displayVal}{unit}</span>
    {/if}
  </div>

  <!-- Context menu -->
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
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 5px;
    user-select: none;
    position: relative;
  }

  .label {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.55rem;
    font-weight: 300;
    letter-spacing: 0.2em;
    text-transform: uppercase;
    color: rgba(130, 110, 85, 0.65);
  }

  .display {
    min-width: 72px;
    height: 28px;
    padding: 0 8px;
    border-radius: 8px;
    background: #e8e0d4;
    box-shadow:
      inset 3px 3px 7px rgba(150, 130, 100, 0.28),
      inset -2px -2px 5px rgba(255, 250, 240, 0.75);
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: text;
    outline: none;
    transition: box-shadow 0.12s;
  }

  .display:focus {
    box-shadow:
      inset 3px 3px 7px rgba(150, 130, 100, 0.38),
      inset -2px -2px 5px rgba(255, 250, 240, 0.85),
      0 0 0 1.5px rgba(160, 130, 90, 0.3);
  }

  .display.editing {
    box-shadow:
      inset 4px 4px 8px rgba(150, 130, 100, 0.35),
      inset -2px -2px 5px rgba(255, 250, 240, 0.8);
  }

  .val-text {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.68rem;
    font-weight: 300;
    color: rgba(120, 95, 65, 0.85);
    letter-spacing: 0.05em;
    pointer-events: none;
  }

  .raw-input {
    width: 100%;
    background: transparent;
    border: none;
    outline: none;
    font-family: 'DM Sans', sans-serif;
    font-size: 0.68rem;
    font-weight: 300;
    color: rgba(100, 75, 45, 0.95);
    letter-spacing: 0.05em;
    text-align: center;
    padding: 0;
  }

  /* ── Context menu (2e) ──────────────────────────────────── */
  .ctx-menu {
    position: fixed;
    z-index: 9998;
    background: #ede6da;
    border-radius: 10px;
    padding: 6px;
    display: flex;
    flex-direction: column;
    gap: 2px;
    box-shadow:
      8px 8px 20px rgba(155,135,105,0.38),
      -6px -6px 16px rgba(255,252,244,0.92);
    min-width: 120px;
  }

  .ctx-item {
    font-family: 'DM Sans', sans-serif;
    font-size: 0.62rem;
    font-weight: 300;
    letter-spacing: 0.08em;
    color: rgba(120, 90, 55, 0.8);
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 6px 10px;
    text-align: left;
    cursor: pointer;
    transition: background 0.12s;
  }

  .ctx-item:hover {
    background: #e2d9cc;
    color: rgba(100, 72, 38, 0.92);
  }
</style>
