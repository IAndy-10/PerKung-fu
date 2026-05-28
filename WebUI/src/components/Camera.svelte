<script lang="ts">
  import { onMount, onDestroy } from 'svelte';
  import { cameraStream } from '../stores/cameraStream';

  let videoEl: HTMLVideoElement;
  let stream: MediaStream | null = null;
  export let active = false;
  export let error  = '';

  async function startCamera() {
    if (!navigator.mediaDevices?.getUserMedia) {
      error = 'NO_MEDIA_DEVICES';
      return;
    }
    try {
      stream = await navigator.mediaDevices.getUserMedia({ video: true, audio: false });
      videoEl.srcObject = stream;
      active = true;
      error  = '';
      cameraStream.set(stream);
    } catch (e: any) {
      error  = e.name ?? 'UNKNOWN';
      active = false;
    }
  }

  function stopCamera() {
    stream?.getTracks().forEach(t => t.stop());
    stream = null;
    active = false;
    if (videoEl) videoEl.srcObject = null;
    cameraStream.set(null);
  }

  onMount(startCamera);
  onDestroy(stopCamera);
</script>

<!-- svelte-ignore a11y-media-has-caption -->
<video
  bind:this={videoEl}
  class:hidden={!active}
  autoplay
  playsinline
  muted
></video>

{#if !active}
  <div class="cam-state">
    {#if error}
      <span class="cam-err">{error}</span>
    {:else}
      <span class="cam-spinner"></span>
    {/if}
  </div>
{/if}

<style>
  video {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    object-fit: cover;
    display: block;
    transform: scaleX(-1);
  }

  .hidden { display: none; }

  .cam-state {
    position: absolute;
    inset: 0;
    display: flex;
    align-items: center;
    justify-content: center;
  }

  .cam-err {
    font-size: 0.42rem;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: rgba(200, 170, 130, 0.45);
  }

  .cam-spinner {
    width: 12px;
    height: 12px;
    border: 1.5px solid rgba(200, 170, 130, 0.15);
    border-top-color: rgba(0, 200, 180, 0.55);
    border-radius: 50%;
    animation: spin 0.9s linear infinite;
  }

  @keyframes spin { to { transform: rotate(360deg); } }
</style>
