<script lang="ts">
  import { onMount, onDestroy } from 'svelte';
  import { cameraStream } from '../stores/cameraStream';
  import { detections } from '../stores/detections';

  // Only surface these COCO labels (add more as needed)
  const ALLOWED = new Set(['banana', 'apple', 'orange', 'cup', 'bottle', 'cell phone']);
  const SCORE_THRESHOLD = 0.4;
  // Throttle detection to 15 fps to limit CPU usage
  const DETECTION_INTERVAL_MS = 1000 / 15;

  let videoEl: HTMLVideoElement;
  let detector: any = null;
  let rafId: number | null = null;
  let lastDetectionTime = 0;
  let status: 'idle' | 'loading' | 'ready' | 'error' = 'idle';

  export let detectorStatus: typeof status = 'idle';
  $: detectorStatus = status;

  function loop(now: number) {
    if (status === 'ready' && detector && videoEl.readyState >= 2) {
      if (now - lastDetectionTime >= DETECTION_INTERVAL_MS) {
        lastDetectionTime = now;
        try {
          const result = detector.detectForVideo(videoEl, now);
          const filtered = (result.detections ?? [])
            .filter((d: any) => {
              const cat = d.categories?.[0];
              return cat && ALLOWED.has(cat.categoryName.toLowerCase()) && cat.score >= SCORE_THRESHOLD;
            })
            .map((d: any) => ({
              label: d.categories[0].categoryName.toLowerCase(),
              score: d.categories[0].score,
              box: {
                left:   d.boundingBox.originX / videoEl.videoWidth,
                top:    d.boundingBox.originY / videoEl.videoHeight,
                right:  (d.boundingBox.originX + d.boundingBox.width)  / videoEl.videoWidth,
                bottom: (d.boundingBox.originY + d.boundingBox.height) / videoEl.videoHeight,
              },
            }));
          detections.set(filtered);
        } catch (_) {
          // swallow per-frame errors
        }
      }
    }
    rafId = requestAnimationFrame(loop);
  }

  async function init(stream: MediaStream) {
    if (status !== 'idle') return;
    status = 'loading';
    try {
      const { ObjectDetector, FilesetResolver } = await import('@mediapipe/tasks-vision');

      const fsr = await FilesetResolver.forVisionTasks(
        'https://cdn.jsdelivr.net/npm/@mediapipe/tasks-vision@0.10.3/wasm'
      );

      detector = await ObjectDetector.createFromOptions(fsr, {
        baseOptions: {
          modelAssetPath:
            'https://storage.googleapis.com/mediapipe-models/' +
            'object_detector/efficientdet_lite0/float16/1/efficientdet_lite0.tflite',
          delegate: 'CPU',
        },
        runningMode: 'VIDEO',
        scoreThreshold: SCORE_THRESHOLD,
        maxResults: 5,
      });

      videoEl.srcObject = stream;
      await new Promise<void>(res => { videoEl.onloadeddata = () => res(); });
      await videoEl.play();

      status = 'ready';
      rafId = requestAnimationFrame(loop);
    } catch (err) {
      console.warn('[ObjectDetector] init failed:', err);
      status = 'error';
    }
  }

  onMount(() => {
    return cameraStream.subscribe(stream => {
      if (stream && status === 'idle') init(stream);
    });
  });

  onDestroy(() => {
    if (rafId !== null) cancelAnimationFrame(rafId);
    detections.set([]);
    try { detector?.close(); } catch (_) {}
  });
</script>

<!-- Hidden video element — MediaPipe reads frames from here -->
<video bind:this={videoEl} style="display:none" muted playsinline></video>
