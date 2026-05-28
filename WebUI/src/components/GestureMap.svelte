<script lang="ts">
  import { createEventDispatcher } from 'svelte';
  const dispatch = createEventDispatcher<{ simulate: string | null }>();

  export let activeGesture: string | null = null;

  let hovered: string | null = null;

  const rows = [
    { id: 'up',         kind: 'single-up',   trigger: 'Index ↑', sub: 'one hand · pointing up',    target: ['Sustain',    'on']  },
    { id: 'down',       kind: 'single-down', trigger: 'Index ↓', sub: 'one hand · pointing down',   target: ['Sustain',    'off'] },
    { id: 'pitch-up',   kind: 'both-up',     trigger: 'Both ↑',  sub: 'two hands · ramping up',     target: ['Pitch Freq', '+']   },
    { id: 'pitch-down', kind: 'both-down',   trigger: 'Both ↓',  sub: 'two hands · ramping down',   target: ['Pitch Freq', '−']   },
  ] as const;

  $: detail = rows.find(r => r.id === hovered) ?? null;

  function enter(id: string) { hovered = id; dispatch('simulate', id); }
  function leave()           { hovered = null; dispatch('simulate', null); }
</script>

<div class="gmap-grid">
  {#each rows as r}
    <div
      class="gmap-card"
      class:hovered={hovered === r.id}
      class:active={activeGesture === r.id}
      role="listitem"
      on:mouseenter={() => enter(r.id)}
      on:mouseleave={leave}
    >
      <div class="gmap-glyph">
        {#if r.kind === 'single-up'}
          <svg width="18" height="18" viewBox="0 0 22 22" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">
            <path d="M11 19 V8 M11 8 L8 11 M11 8 L14 11" /><circle cx="11" cy="5" r="1.4" fill="currentColor" stroke="none" />
          </svg>
        {:else if r.kind === 'single-down'}
          <svg width="18" height="18" viewBox="0 0 22 22" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">
            <path d="M11 3 V14 M11 14 L8 11 M11 14 L14 11" /><circle cx="11" cy="17" r="1.4" fill="currentColor" stroke="none" />
          </svg>
        {:else if r.kind === 'both-up'}
          <svg width="18" height="18" viewBox="0 0 22 22" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
            <path d="M6 19 V9 M6 9 L4 11 M6 9 L8 11" /><path d="M16 19 V9 M16 9 L14 11 M16 9 L18 11" />
            <circle cx="6" cy="6" r="1.2" fill="currentColor" stroke="none" /><circle cx="16" cy="6" r="1.2" fill="currentColor" stroke="none" />
          </svg>
        {:else}
          <svg width="18" height="18" viewBox="0 0 22 22" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
            <path d="M6 3 V13 M6 13 L4 11 M6 13 L8 11" /><path d="M16 3 V13 M16 13 L14 11 M16 13 L18 11" />
            <circle cx="6" cy="16" r="1.2" fill="currentColor" stroke="none" /><circle cx="16" cy="16" r="1.2" fill="currentColor" stroke="none" />
          </svg>
        {/if}
      </div>
      <div class="gmap-trigger">{r.trigger}</div>
    </div>
  {/each}
</div>

<div class="gmap-detail">
  {#if detail}
    <div class="gmap-detail-sub">{detail.sub}</div>
    <div class="gmap-detail-map">
      <span class="gmap-detail-target">{detail.target[0]}</span>
      <span class="gmap-arrow">→</span>
      <span class="gmap-chip">{detail.target[1]}</span>
    </div>
  {:else}
    <div class="gmap-detail-placeholder">hover a gesture to inspect mapping</div>
  {/if}
</div>

<style>
  .gmap-grid { display: flex; flex-direction: column; gap: 6px; }

  .gmap-card {
    display: flex; align-items: center; gap: 10px;
    padding: 8px 10px; border-radius: 9px;
    border: 1px solid var(--glass-border);
    background: var(--glass-bg);
    backdrop-filter: blur(8px); -webkit-backdrop-filter: blur(8px);
    transition: all 0.22s ease; cursor: default;
  }
  .gmap-card:hover, .gmap-card.hovered, .gmap-card.active {
    border-color: var(--accent-line); background: rgba(255,182,72,0.06);
  }
  .gmap-glyph {
    width: 26px; height: 26px; flex-shrink: 0; border-radius: 7px;
    background: var(--glass-bg-hi); border: 1px solid var(--glass-border);
    display: flex; align-items: center; justify-content: center;
    color: var(--text); transition: all 0.22s ease;
  }
  .gmap-card:hover .gmap-glyph, .gmap-card.hovered .gmap-glyph, .gmap-card.active .gmap-glyph {
    border-color: var(--accent-line); color: var(--accent);
  }
  .gmap-trigger {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.58rem; font-weight: 400; letter-spacing: 0.22em;
    text-transform: uppercase; color: var(--text);
  }
  .gmap-detail {
    margin-top: 14px; padding: 10px 12px;
    border-top: 1px solid var(--glass-border);
    min-height: 56px; display: flex; flex-direction: column; gap: 6px;
  }
  .gmap-detail-sub {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.52rem; letter-spacing: 0.22em;
    text-transform: uppercase; color: var(--text-lo); line-height: 1.4;
  }
  .gmap-detail-map { display: flex; align-items: center; gap: 8px; }
  .gmap-detail-target {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.56rem; letter-spacing: 0.22em;
    text-transform: uppercase; color: var(--text);
  }
  .gmap-detail-placeholder {
    font-family: 'JetBrains Mono', monospace;
    font-size: 0.5rem; letter-spacing: 0.24em;
    text-transform: uppercase; color: var(--text-xlo);
    text-align: center; margin: auto 0;
  }
  .gmap-arrow { color: var(--accent); opacity: 0.7; font-family: 'JetBrains Mono', monospace; }
  .gmap-chip {
    display: inline-block; padding: 2px 8px; border-radius: 5px;
    background: rgba(255,182,72,0.08); border: 1px solid var(--accent-line);
    color: var(--accent); font-family: 'JetBrains Mono', monospace;
    font-size: 0.5rem; letter-spacing: 0.16em;
  }
</style>
