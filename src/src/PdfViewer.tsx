import { useEffect, useRef, useState } from "react";
import * as pdfjsLib from "pdfjs-dist";
import type { PDFDocumentProxy, RenderTask } from "pdfjs-dist";
import pdfWorker from "pdfjs-dist/build/pdf.worker.mjs?url";

pdfjsLib.GlobalWorkerOptions.workerSrc = pdfWorker;

const ZOOM_STEP = 0.2;
const MIN_ZOOM = 0.4;
const MAX_ZOOM = 3;

type PdfViewerProps = {
  file: File | null;
};

export default function PdfViewer({ file }: PdfViewerProps) {
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const stageRef = useRef<HTMLDivElement | null>(null);
  const isPointerOverStageRef = useRef(false);
  const isSpaceDownRef = useRef(false);
  const isPanningRef = useRef(false);
  const dragStartRef = useRef({ x: 0, y: 0, scrollLeft: 0, scrollTop: 0 });
  const renderTaskRef = useRef<RenderTask | null>(null);
  const [pdf, setPdf] = useState<PDFDocumentProxy | null>(null);
  const [pageNumber, setPageNumber] = useState(1);
  const [zoom, setZoom] = useState(1);
  const [isPanMode, setIsPanMode] = useState(false);
  const [isPanning, setIsPanning] = useState(false);
  const [status, setStatus] = useState("Choose a PDF to preview its original pages.");

  function changeZoom(nextZoom: number) {
    setZoom(Math.min(MAX_ZOOM, Math.max(MIN_ZOOM, nextZoom)));
  }

  useEffect(() => {
    let isCancelled = false;

    async function loadPdf() {
      setPdf(null);
      setPageNumber(1);
      setZoom(1);

      if (!file) {
        setStatus("Choose a PDF to preview its original pages.");
        return;
      }

      setStatus("Loading preview...");
      const data = await file.arrayBuffer();
      const loadedPdf = await pdfjsLib.getDocument({ data }).promise;

      if (!isCancelled) {
        setPdf(loadedPdf);
        setPageNumber(1);
        setStatus("");
      } else {
        loadedPdf.destroy();
      }
    }

    loadPdf().catch((error: unknown) => {
      setStatus(error instanceof Error ? error.message : "Could not load PDF preview.");
    });

    return () => {
      isCancelled = true;
    };
  }, [file]);

  useEffect(() => {
    function handleKeyDown(event: KeyboardEvent) {
      const stage = stageRef.current;
      if (!stage) {
        return;
      }

      const activeElement = document.activeElement;
      const isStageFocused = activeElement ? stage.contains(activeElement) : false;
      const shouldHandleStageShortcut = isStageFocused || isPointerOverStageRef.current;

      if (!shouldHandleStageShortcut) {
        return;
      }

      if (event.code === "Space") {
        event.preventDefault();
        event.stopPropagation();
        isSpaceDownRef.current = true;
        setIsPanMode(true);
        return;
      }

      if (!event.ctrlKey && !event.metaKey) {
        return;
      }

      if (event.key === "+" || event.key === "=" || event.code === "NumpadAdd") {
        event.preventDefault();
        event.stopPropagation();
        changeZoom(zoom + ZOOM_STEP);
      } else if (event.key === "-" || event.code === "NumpadSubtract") {
        event.preventDefault();
        event.stopPropagation();
        changeZoom(zoom - ZOOM_STEP);
      } else if (event.key === "0" || event.code === "Numpad0") {
        event.preventDefault();
        event.stopPropagation();
        changeZoom(1);
      }
    }

    function handleKeyUp(event: KeyboardEvent) {
      if (event.code !== "Space") {
        return;
      }

      isSpaceDownRef.current = false;
      isPanningRef.current = false;
      setIsPanMode(false);
      setIsPanning(false);
    }

    window.addEventListener("keydown", handleKeyDown, { capture: true });
    window.addEventListener("keyup", handleKeyUp, { capture: true });
    return () => {
      window.removeEventListener("keydown", handleKeyDown, { capture: true });
      window.removeEventListener("keyup", handleKeyUp, { capture: true });
    };
  }, [zoom]);

  useEffect(() => {
    let isCancelled = false;

    async function renderPage() {
      const canvas = canvasRef.current;
      if (!pdf || !canvas) {
        return;
      }

      renderTaskRef.current?.cancel();
      const page = await pdf.getPage(pageNumber);
      const baseViewport = page.getViewport({ scale: 1 });
      const maxWidth = Math.min(window.innerWidth - 80, 820);
      const fitScale = Math.min(maxWidth / baseViewport.width, 1.5);
      const scale = fitScale * zoom;
      const viewport = page.getViewport({ scale });
      const context = canvas.getContext("2d");

      if (!context) {
        setStatus("Canvas is not available in this browser.");
        return;
      }

      canvas.width = Math.floor(viewport.width);
      canvas.height = Math.floor(viewport.height);

      const renderTask = page.render({ canvas, canvasContext: context, viewport });
      renderTaskRef.current = renderTask;

      try {
        await renderTask.promise;
        if (!isCancelled) {
          setStatus("");
        }
      } catch (error: unknown) {
        if (!isCancelled && !(error instanceof Error && error.name === "RenderingCancelledException")) {
          setStatus(error instanceof Error ? error.message : "Could not render page.");
        }
      }
    }

    renderPage();

    return () => {
      isCancelled = true;
      renderTaskRef.current?.cancel();
    };
  }, [pageNumber, pdf, zoom]);

  const pageCount = pdf?.numPages ?? 0;
  const canGoBack = pageNumber > 1;
  const canGoForward = pageCount > 0 && pageNumber < pageCount;
  const stageClassName = [
    "page-stage",
    isPanMode ? "is-pan-mode" : "",
    isPanning ? "is-panning" : ""
  ]
    .filter(Boolean)
    .join(" ");

  return (
    <section className="viewer-panel">
      <div className="viewer-header">
        <div>
          <h2>Original page</h2>
          <p>{file ? file.name : "No PDF selected"}</p>
        </div>
        <div className="page-controls">
          <button
            disabled={!canGoBack}
            type="button"
            onClick={() => setPageNumber((current) => Math.max(1, current - 1))}
          >
            Previous
          </button>
          <span>
            Page {pageCount ? pageNumber : "-"} of {pageCount || "-"}
          </span>
          <button
            disabled={!canGoForward}
            type="button"
            onClick={() =>
              setPageNumber((current) => Math.min(pageCount, current + 1))
            }
          >
            Next
          </button>
          <strong>{Math.round(zoom * 100)}%</strong>
        </div>
      </div>

      <div
        className={stageClassName}
        ref={stageRef}
        tabIndex={0}
        onMouseEnter={() => {
          isPointerOverStageRef.current = true;
          stageRef.current?.focus({ preventScroll: true });
        }}
        onMouseLeave={() => {
          isPointerOverStageRef.current = false;
        }}
        onClick={() => stageRef.current?.focus({ preventScroll: true })}
        onPointerDown={(event) => {
          const stage = stageRef.current;
          if (!stage || !isSpaceDownRef.current || event.button !== 0) {
            return;
          }

          event.preventDefault();
          stage.focus({ preventScroll: true });
          stage.setPointerCapture(event.pointerId);
          dragStartRef.current = {
            x: event.clientX,
            y: event.clientY,
            scrollLeft: stage.scrollLeft,
            scrollTop: stage.scrollTop
          };
          isPanningRef.current = true;
          setIsPanning(true);
        }}
        onPointerMove={(event) => {
          const stage = stageRef.current;
          if (!stage || !isPanningRef.current) {
            return;
          }

          event.preventDefault();
          const start = dragStartRef.current;
          stage.scrollLeft = start.scrollLeft - (event.clientX - start.x);
          stage.scrollTop = start.scrollTop - (event.clientY - start.y);
        }}
        onPointerUp={(event) => {
          if (stageRef.current?.hasPointerCapture(event.pointerId)) {
            stageRef.current.releasePointerCapture(event.pointerId);
          }
          isPanningRef.current = false;
          setIsPanning(false);
        }}
        onPointerCancel={() => {
          isPanningRef.current = false;
          setIsPanning(false);
        }}
        aria-label="PDF page preview. Use Ctrl plus, Ctrl minus, and Ctrl zero to zoom this preview."
      >
        {status && <p className="viewer-status">{status}</p>}
        <div className="page-canvas-wrap">
          <canvas
            className={pdf ? "pdf-canvas is-visible" : "pdf-canvas"}
            ref={canvasRef}
          />
        </div>
      </div>
    </section>
  );
}
