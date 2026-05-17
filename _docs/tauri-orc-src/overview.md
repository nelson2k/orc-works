# Overview

The Tauri app at [tauri-orc-src/](../../tauri-orc-src/) is a standard Tauri 2 scaffold (React + Vite + TypeScript on the frontend, Rust on the backend) plus a `marker-code/` sidecar that runs the Python marker server.

The internal `src/` is the React app and `src-tauri/` is the Rust crate — those names come from the Tauri template; they are not paths relative to the repo root.

## Layout

```
tauri-orc-src/
├── .vscode/extensions.json    VS Code recommendations (Tauri + rust-analyzer)
├── .gitignore                 node_modules, dist, logs, editor cruft
├── README.md                  stock Tauri+React+TS template blurb
├── index.html                 Vite entry; mounts /src/main.tsx
├── package.json               npm scripts + JS deps
├── package-lock.json          npm exact-version lockfile
├── tsconfig.json              strict TS, react-jsx, bundler resolution
├── tsconfig.node.json         node-side tsconfig
├── vite.config.ts             Vite dev server on port 1420 (Tauri-fixed)
├── public/                    static assets served by Vite (tauri.svg, vite.svg)
├── src/                       React frontend → see frontend.md
├── src-tauri/                 Rust Tauri shell → see tauri-shell.md
└── marker-code/               Python marker sidecar → see marker-sidecar.md
```

## How the three pieces talk

1. On `App` mount, the React app calls `marker_start()` over Tauri `invoke()`. The Rust side spawns the Python sidecar at `127.0.0.1:7423`.
2. The React app polls `http://127.0.0.1:7423/health` until the sidecar responds, then sets `serverStatus = "ready"` and enables the convert button.
3. On submit, the React app POSTs the PDF + form fields to `http://127.0.0.1:7423/convert`. The sidecar runs marker and returns `{ markdown }`.
4. On window close, the Rust exit handler kills the Python child.

Vite runs separately on `127.0.0.1:1420` and only serves the frontend during dev. The production build is bundled into `frontendDist: ../dist` and served from the Tauri shell — Vite is not in the loop at runtime.

## Port allocation

| Port | Process | Owner |
| --- | --- | --- |
| 1420 | Vite dev server (frontend HMR) | `npm run dev` |
| 7423 | uvicorn / FastAPI marker sidecar | spawned by `marker_start` |
