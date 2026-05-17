import { FormEvent, useEffect, useRef, useState } from "react";
import PdfFilePicker from "./PdfFilePicker";
import PdfViewer from "./PdfViewer";
import { convertPdf, startMarker, stopMarker, waitForReady } from "./markerClient";

type ConvertResponse = {
  documentName: string;
  markdown: string;
};

type ServerStatus = "starting" | "ready" | "error";

const openAiModels = [
  { value: "gpt-4o-mini", label: "GPT-4o mini" },
  { value: "gpt-4o", label: "GPT-4o" },
  { value: "gpt-4.1-nano", label: "GPT-4.1 nano" },
  { value: "gpt-4.1-mini", label: "GPT-4.1 mini" },
  { value: "gpt-4.1", label: "GPT-4.1" },
  { value: "gpt-5-nano", label: "GPT-5 nano" },
  { value: "gpt-5-mini", label: "GPT-5 mini" },
  { value: "gpt-5", label: "GPT-5" }
];

export default function App() {
  const abortControllerRef = useRef<AbortController | null>(null);
  const [file, setFile] = useState<File | null>(null);
  const [pageRange, setPageRange] = useState("");
  const [model, setModel] = useState("gpt-4o-mini");
  const [noLlm, setNoLlm] = useState(false);
  const [fullVram, setFullVram] = useState(false);
  const [result, setResult] = useState<ConvertResponse | null>(null);
  const [error, setError] = useState("");
  const [isConverting, setIsConverting] = useState(false);
  const [serverStatus, setServerStatus] = useState<ServerStatus>("starting");
  const [serverError, setServerError] = useState("");

  useEffect(() => {
    let isCancelled = false;

    async function bootMarker() {
      try {
        await startMarker();
        await waitForReady();
        if (!isCancelled) {
          setServerStatus("ready");
        }
      } catch (err) {
        if (!isCancelled) {
          setServerStatus("error");
          setServerError(err instanceof Error ? err.message : "Failed to start marker server.");
        }
      }
    }

    bootMarker();

    return () => {
      isCancelled = true;
      stopMarker().catch(() => {
        // best-effort shutdown on unmount; Tauri also kills on app exit
      });
    };
  }, []);

  async function handleSubmit(event: FormEvent<HTMLFormElement>) {
    event.preventDefault();
    if (!file) {
      setError("Choose a PDF first.");
      return;
    }

    setError("");
    setResult(null);
    setIsConverting(true);
    abortControllerRef.current?.abort();
    abortControllerRef.current = new AbortController();

    try {
      const payload = await convertPdf(file, {
        pageRange,
        model,
        noLlm,
        fullVram,
        signal: abortControllerRef.current.signal,
      });
      setResult({ documentName: file.name, markdown: payload.markdown });
    } catch (err) {
      if (err instanceof DOMException && err.name === "AbortError") {
        setError("Conversion stopped.");
        return;
      }
      setError(err instanceof Error ? err.message : "Conversion failed.");
    } finally {
      abortControllerRef.current = null;
      setIsConverting(false);
    }
  }

  function handleStopConversion() {
    abortControllerRef.current?.abort();
  }

  return (
    <main className="app-shell">
      <section className="workspace">
        <div className="toolbar">
          <div>
            <h1>OCR Works</h1>
            <p>Upload a PDF and convert it with the local Marker backend.</p>
          </div>
          <div className={`server-status server-status-${serverStatus}`}>
            {serverStatus === "starting" && "Starting marker server..."}
            {serverStatus === "ready" && "Marker server ready"}
            {serverStatus === "error" && `Marker server failed: ${serverError}`}
          </div>
        </div>

        <PdfViewer file={file} />

        <form className="panel" onSubmit={handleSubmit}>
          <label>
            PDF file
            <PdfFilePicker file={file} onFileChange={setFile} />
          </label>

          <div className="grid">
            <label>
              Page range
              <input
                placeholder="all, 1-10, or 1,6-11"
                value={pageRange}
                onChange={(event) => setPageRange(event.target.value)}
              />
            </label>

            <label>
              OpenAI model
              <select
                value={model}
                onChange={(event) => setModel(event.target.value)}
                disabled={noLlm}
              >
                {openAiModels.map((option) => (
                  <option key={option.value} value={option.value}>
                    {option.label}
                  </option>
                ))}
              </select>
            </label>
          </div>

          <div className="checks">
            <label>
              <input
                checked={noLlm}
                type="checkbox"
                onChange={(event) => setNoLlm(event.target.checked)}
              />
              Disable LLM processors
            </label>

            <label>
              <input
                checked={fullVram}
                type="checkbox"
                onChange={(event) => setFullVram(event.target.checked)}
              />
              Use full VRAM settings
            </label>
          </div>

          <div className="action-row">
            <button disabled={isConverting || serverStatus !== "ready"} type="submit">
              {isConverting ? "Converting..." : "Convert PDF"}
            </button>
            {isConverting && (
              <button className="stop-button" type="button" onClick={handleStopConversion}>
                Stop
              </button>
            )}
          </div>
        </form>

        {error && <p className="error">{error}</p>}

        {result && (
          <section className="result">
            <div className="result-header">
              <h2>{result.documentName}</h2>
            </div>
            <pre>{result.markdown}</pre>
          </section>
        )}
      </section>
    </main>
  );
}
