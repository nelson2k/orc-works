# OCR Works

Web app scaffold for converting PDFs with the local Marker runtime in `marker-code`.

## Setup

Install the backend web dependencies into the existing Marker venv.

Windows:

```powershell
.\setup-backend.ps1
```

Linux:

```bash
chmod +x setup-backend.sh run-backend.sh run-frontend.sh
./setup-backend.sh
```

Install the frontend dependencies:

```bash
npm install --prefix frontend
```

## Run

Start both the API and frontend.

Windows:

```powershell
.\run-app.ps1
```

Start the API.

Windows:

```powershell
.\run-backend.ps1
```

Linux:

```bash
./run-backend.sh
```

Start the React app.

Windows:

```powershell
.\run-frontend.ps1
```

Linux:

```bash
./run-frontend.sh
```

The frontend runs at `http://localhost:5173` and proxies API calls to `http://127.0.0.1:8000`.

## Expected Layout

The app expects the Marker virtual environment at:

- Windows: `marker-code\venv\Scripts\python.exe`
- Linux: `marker-code/venv/bin/python`
