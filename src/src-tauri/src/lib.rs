use std::path::PathBuf;
use std::process::{Child, Command, Stdio};
use std::sync::Mutex;

use tauri::{Manager, RunEvent, State};

const MARKER_PORT: u16 = 7423;

struct MarkerState {
    child: Mutex<Option<Child>>,
}

fn app_root() -> Result<PathBuf, String> {
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("..")
        .canonicalize()
        .map_err(|e| format!("resolve app root: {e}"))
}

#[tauri::command]
fn marker_server_url() -> String {
    format!("http://127.0.0.1:{MARKER_PORT}")
}

#[tauri::command]
fn marker_status(state: State<'_, MarkerState>) -> String {
    let mut guard = state.child.lock().unwrap();
    match guard.as_mut() {
        None => "stopped".into(),
        Some(child) => match child.try_wait() {
            Ok(None) => "running".into(),
            Ok(Some(_)) => {
                *guard = None;
                "stopped".into()
            }
            Err(e) => format!("error: {e}"),
        },
    }
}

#[tauri::command]
fn marker_start(state: State<'_, MarkerState>) -> Result<String, String> {
    let mut guard = state.child.lock().map_err(|e| e.to_string())?;

    if let Some(child) = guard.as_mut() {
        if matches!(child.try_wait(), Ok(None)) {
            return Ok(format!("already running on port {MARKER_PORT}"));
        }
    }

    let root = app_root()?;
    let python = root
        .join("marker-code")
        .join("venv")
        .join("Scripts")
        .join("python.exe");
    let cwd = root.join("marker-code");

    if !python.exists() {
        return Err(format!("python not found: {}", python.display()));
    }
    if !cwd.join("server.py").exists() {
        return Err(format!("server.py not found in {}", cwd.display()));
    }

    let child = Command::new(&python)
        .arg("-m")
        .arg("uvicorn")
        .arg("server:app")
        .arg("--host")
        .arg("127.0.0.1")
        .arg("--port")
        .arg(MARKER_PORT.to_string())
        .current_dir(&cwd)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|e| format!("spawn failed: {e}"))?;

    *guard = Some(child);
    Ok(format!("started on port {MARKER_PORT}"))
}

#[tauri::command]
fn marker_stop(state: State<'_, MarkerState>) -> Result<String, String> {
    let mut guard = state.child.lock().map_err(|e| e.to_string())?;
    if let Some(mut child) = guard.take() {
        child.kill().map_err(|e| e.to_string())?;
        let _ = child.wait();
        Ok("stopped".into())
    } else {
        Ok("not running".into())
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .manage(MarkerState {
            child: Mutex::new(None),
        })
        .invoke_handler(tauri::generate_handler![
            marker_server_url,
            marker_status,
            marker_start,
            marker_stop,
        ])
        .build(tauri::generate_context!())
        .expect("error while building tauri application")
        .run(|app_handle, event| {
            if matches!(event, RunEvent::ExitRequested { .. } | RunEvent::Exit) {
                if let Some(state) = app_handle.try_state::<MarkerState>() {
                    if let Ok(mut guard) = state.child.lock() {
                        if let Some(mut child) = guard.take() {
                            let _ = child.kill();
                            let _ = child.wait();
                        }
                    }
                }
            }
        });
}
