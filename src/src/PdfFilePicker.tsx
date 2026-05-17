import { ChangeEvent, useEffect, useRef, useState } from "react";
import * as pdfjsLib from "pdfjs-dist";
import type { RenderTask } from "pdfjs-dist";
import pdfWorker from "pdfjs-dist/build/pdf.worker.mjs?url";

pdfjsLib.GlobalWorkerOptions.workerSrc = pdfWorker;

type PdfFilePickerProps = {
  file: File | null;
  onFileChange: (file: File | null) => void;
};

export default function PdfFilePicker({ file, onFileChange }: PdfFilePickerProps) {
  const inputRef = useRef<HTMLInputElement | null>(null);
  const canvasRef = useRef<HTMLCanvasElement | null>(null);
  const renderTaskRef = useRef<RenderTask | null>(null);
  const [thumbnailStatus, setThumbnailStatus] = useState("");

  function openPicker() {
    inputRef.current?.click();
  }

  function handleFileChange(event: ChangeEvent<HTMLInputElement>) {
    onFileChange(event.target.files?.[0] ?? null);
  }

  useEffect(() => {
    let isCancelled = false;

    async function renderThumbnail() {
      const canvas = canvasRef.current;
      if (!file || !canvas) {
        setThumbnailStatus("");
        return;
      }

      setThumbnailStatus("Loading thumbnail...");
      renderTaskRef.current?.cancel();

      const data = await file.arrayBuffer();
      const pdf = await pdfjsLib.getDocument({ data }).promise;

      try {
        const page = await pdf.getPage(1);
        const baseViewport = page.getViewport({ scale: 1 });
        const scale = Math.min(120 / baseViewport.width, 160 / baseViewport.height);
        const viewport = page.getViewport({ scale });
        const context = canvas.getContext("2d");

        if (!context) {
          setThumbnailStatus("Thumbnail unavailable.");
          return;
        }

        canvas.width = Math.floor(viewport.width);
        canvas.height = Math.floor(viewport.height);

        const renderTask = page.render({ canvas, canvasContext: context, viewport });
        renderTaskRef.current = renderTask;
        await renderTask.promise;

        if (!isCancelled) {
          setThumbnailStatus("");
        }
      } finally {
        pdf.destroy();
      }
    }

    renderThumbnail().catch((error: unknown) => {
      if (!(error instanceof Error && error.name === "RenderingCancelledException")) {
        setThumbnailStatus("Thumbnail unavailable.");
      }
    });

    return () => {
      isCancelled = true;
      renderTaskRef.current?.cancel();
    };
  }, [file]);

  return (
    <div className="pdf-picker">
      <input
        ref={inputRef}
        accept="application/pdf"
        className="file-input"
        type="file"
        onChange={handleFileChange}
      />

      {file ? (
        <div className="selected-file">
          <div className="thumbnail-frame">
            <canvas ref={canvasRef} />
            {thumbnailStatus && <span>{thumbnailStatus}</span>}
          </div>
          <div className="selected-file-details">
            <strong>{file.name}</strong>
            <button type="button" onClick={openPicker}>
              Change PDF
            </button>
          </div>
        </div>
      ) : (
        <button className="choose-pdf-button" type="button" onClick={openPicker}>
          Choose PDF
        </button>
      )}
    </div>
  );
}
