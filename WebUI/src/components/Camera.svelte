<script lang="ts">
  import { onMount, onDestroy } from 'svelte';
  import { cameraStream } from '../stores/cameraStream';
  import { detections } from '../stores/detections';
  import type { Detection } from '../stores/detections';

  let videoEl: HTMLVideoElement;
  let canvasEl: HTMLCanvasElement;
  let stream: MediaStream | null = null;
  export let active = false;
  export let error  = '';

  // Cover-crop compensation — keeps bbox coordinates pixel-aligned with the displayed video
  let coverScale = 1;
  let coverOX    = 0;
  let coverOY    = 0;

  function updateCoverTransform() {
    if (!canvasEl || !videoEl?.videoWidth) return;
    const rect = canvasEl.getBoundingClientRect();
    const dW = rect.width;
    const dH = rect.height;
    if (dW === 0 || dH === 0) return;
    canvasEl.width  = dW;
    canvasEl.height = dH;
    const vW = videoEl.videoWidth;
    const vH = videoEl.videoHeight;
    coverScale = Math.max(dW / vW, dH / vH);
    coverOX    = (dW - vW * coverScale) / 2;
    coverOY    = (dH - vH * coverScale) / 2;
  }

  // Draw bounding boxes whenever the detections store updates
  function drawBBoxes(dets: Detection[]) {
    if (!canvasEl) return;
    const ctx = canvasEl.getContext('2d');
    if (!ctx) return;
    ctx.clearRect(0, 0, canvasEl.width, canvasEl.height);
    if (!videoEl?.videoWidth) return;

    for (const d of dets) {
      const x = d.box.left   * videoEl.videoWidth  * coverScale + coverOX;
      const y = d.box.top    * videoEl.videoHeight * coverScale + coverOY;
      const w = (d.box.right  - d.box.left) * videoEl.videoWidth  * coverScale;
      const h = (d.box.bottom - d.box.top)  * videoEl.videoHeight * coverScale;

      // Box
      ctx.strokeStyle = 'rgba(0, 200, 180, 0.85)';
      ctx.lineWidth = 1.5;
      ctx.strokeRect(x, y, w, h);

      // Corner accent — top-left bracket
      ctx.strokeStyle = 'rgba(0, 200, 180, 1)';
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.moveTo(x, y + 10); ctx.lineTo(x, y); ctx.lineTo(x + 10, y);
      ctx.stroke();
      // bottom-right bracket
      ctx.beginPath();
      ctx.moveTo(x + w - 10, y + h); ctx.lineTo(x + w, y + h); ctx.lineTo(x + w, y + h - 10);
      ctx.stroke();

      // Label pill
      ctx.font = '9px "JetBrains Mono", monospace';
      const label = `${d.label.toUpperCase()} ${(d.score * 100).toFixed(0)}%`;
      const tw = ctx.measureText(label).width;
      ctx.fillStyle = 'rgba(2, 8, 10, 0.75)';
      ctx.fillRect(x, y - 16, tw + 10, 16);
      ctx.fillStyle = 'rgba(0, 200, 180, 0.95)';
      ctx.fillText(label, x + 5, y - 4);
    }
  }

  $: drawBBoxes($detections);

  async function startCamera() {
    if (!navigator.mediaDevices?.getUserMedia) {
      error = 'NO_MEDIA_DEVICES';
      return;
    }
    try {
      stream = await navigator.mediaDevices.getUserMedia({ video: true, audio: false });
      videoEl.srcObject = stream;
      videoEl.addEventListener('loadedmetadata', updateCoverTransform, { once: true });
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

  onMount(() => {
    startCamera();
    const resizeObs = new ResizeObserver(updateCoverTransform);
    resizeObs.observe(canvasEl);
    return () => resizeObs.disconnect();
  });

  onDestroy(stopCamera);
</script>

<!-- svelte-ignore a11y-media-has-caption -->
<video bind:this={videoEl} class:hidden={!active} autoplay playsinline muted></video>

<!-- Bbox overlay — scaleX(-1) mirrors coordinates to match displayed video -->
<canvas bind:this={canvasEl} class="bbox-overlay"></canvas>

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

  .bbox-overlay {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    transform: scaleX(-1);
    pointer-events: none;
    z-index: 1;
  }

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
