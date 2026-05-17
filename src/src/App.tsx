import { FormEvent, useRef, useState } from "react";
import PdfFilePicker from "./PdfFilePicker";
import PdfViewer from "./PdfViewer";

type ConvertResponse = {
  documentName: string;
  markdown: string;
  markdownUrl: string;
  metadataUrl: string;
};

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

    const body = new FormData();
    body.append("pdf", file);
    body.append("page_range", pageRange);
    body.append("model", model);
    body.append("no_llm", String(noLlm));
    body.append("full_vram", String(fullVram));

    try {
      const response = await fetch("/api/convert", {
        method: "POST",
        body,
        signal: abortControllerRef.current.signal
      });
      const payload = await response.json();
      if (!response.ok) {
        throw new Error(payload.detail ?? "Conversion failed.");
      }
      setResult(payload);
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
            <button disabled={isConverting} type="submit">
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
              <div>
                <a href={result.markdownUrl}>Markdown</a>
                <a href={result.metadataUrl}>Metadata</a>
              </div>
            </div>
            <pre>{result.markdown}</pre>
          </section>
        )}
      </section>
    </main>
  );
}
