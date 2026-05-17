import { invoke } from '@tauri-apps/api/core'

export type MarkerStatus = 'stopped' | 'running' | string

export function startMarker(): Promise<string> {
  return invoke('marker_start')
}

export function stopMarker(): Promise<string> {
  return invoke('marker_stop')
}

export function markerStatus(): Promise<MarkerStatus> {
  return invoke('marker_status')
}

export function markerUrl(): Promise<string> {
  return invoke('marker_server_url')
}

export async function waitForReady(timeoutMs = 60_000): Promise<void> {
  const base = await markerUrl()
  const deadline = Date.now() + timeoutMs
  while (Date.now() < deadline) {
    try {
      const res = await fetch(`${base}/health`)
      if (res.ok) return
    } catch {
      // not up yet
    }
    await new Promise((r) => setTimeout(r, 500))
  }
  throw new Error(`marker server did not respond within ${timeoutMs}ms`)
}

export type ConvertOptions = {
  pageRange?: string
  model?: string
  noLlm?: boolean
  fullVram?: boolean
  signal?: AbortSignal
}

export async function convertPdf(
  file: File,
  options: ConvertOptions = {},
): Promise<{ markdown: string }> {
  const base = await markerUrl()
  const form = new FormData()
  form.append('file', file)
  if (options.pageRange) form.append('page_range', options.pageRange)
  if (options.model) form.append('model', options.model)
  if (options.noLlm !== undefined) form.append('no_llm', String(options.noLlm))
  if (options.fullVram !== undefined) form.append('full_vram', String(options.fullVram))
  const res = await fetch(`${base}/convert`, {
    method: 'POST',
    body: form,
    signal: options.signal,
  })
  if (!res.ok) throw new Error(`marker /convert failed: ${res.status} ${await res.text()}`)
  return res.json()
}
