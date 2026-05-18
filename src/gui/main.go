package main

import (
	"bufio"
	"bytes"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"image"
	"image/color"
	_ "image/png"
	"io"
	"os/exec"
	"path/filepath"
	"sync"
	"time"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/canvas"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/widget"
	nfd "github.com/sqweek/dialog"
)

type Worker struct {
	mu     sync.Mutex
	cmd    *exec.Cmd
	stdin  io.WriteCloser
	stdout *bufio.Scanner
}

func (w *Worker) ensure() error {
	if w.cmd != nil {
		return nil
	}
	pythonExe := filepath.Join("..", "venv", "Scripts", "python.exe")
	workerPy := filepath.Join("..", "worker.py")
	cmd := exec.Command(pythonExe, workerPy)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		return err
	}
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return err
	}
	if err := cmd.Start(); err != nil {
		return err
	}
	scanner := bufio.NewScanner(stdout)
	scanner.Buffer(make([]byte, 0, 64*1024), 64*1024*1024)
	w.cmd = cmd
	w.stdin = stdin
	w.stdout = scanner
	return nil
}

func (w *Worker) request(req map[string]any, onProgress func(map[string]any)) (map[string]any, error) {
	w.mu.Lock()
	defer w.mu.Unlock()
	if err := w.ensure(); err != nil {
		return nil, err
	}
	b, err := json.Marshal(req)
	if err != nil {
		return nil, err
	}
	if _, err := w.stdin.Write(append(b, '\n')); err != nil {
		return nil, err
	}
	for {
		if !w.stdout.Scan() {
			if e := w.stdout.Err(); e != nil {
				return nil, e
			}
			return nil, fmt.Errorf("worker closed stdout")
		}
		var resp map[string]any
		if err := json.Unmarshal(w.stdout.Bytes(), &resp); err != nil {
			return nil, fmt.Errorf("decode worker reply: %w (raw: %q)", err, w.stdout.Bytes())
		}
		if t, _ := resp["type"].(string); t == "progress" {
			if onProgress != nil {
				onProgress(resp)
			}
			continue
		}
		return resp, nil
	}
}

func (w *Worker) shutdown() {
	w.mu.Lock()
	defer w.mu.Unlock()
	if w.cmd == nil {
		return
	}
	_, _ = w.stdin.Write([]byte(`{"cmd":"quit"}` + "\n"))
	_ = w.stdin.Close()
	_ = w.cmd.Wait()
}

func runMetricsLoop(stop <-chan struct{}, cpuBar, ramBar, gpuBar, vramBar, tempBar *vBar) {
	_ = collect()
	tick := time.NewTicker(1 * time.Second)
	defer tick.Stop()

	apply := func(s sample) {
		cpuBar.set(s.cpuPct/100, fmt.Sprintf("%.0f%%", s.cpuPct))
		ramBar.set(s.ramPct/100, fmt.Sprintf("%.1fG", s.ramUsedGB))
		if s.hasGPU {
			gpuBar.set(s.gpuPct/100, fmt.Sprintf("%.0f%%", s.gpuPct))
			vramBar.set(s.vramPct/100, fmt.Sprintf("%.1fG", s.vramUsedMB/1024))
			tempBar.set(s.tempC/100, fmt.Sprintf("%.0f°", s.tempC))
		} else {
			gpuBar.set(0, "n/a")
			vramBar.set(0, "n/a")
			tempBar.set(0, "n/a")
		}
	}

	for {
		select {
		case <-stop:
			return
		case <-tick.C:
			s := collect()
			fyne.Do(func() { apply(s) })
		}
	}
}

func main() {
	a := app.New()
	win := a.NewWindow("OCR Works")

	worker := &Worker{}
	defer worker.shutdown()

	imgCanvas := canvas.NewImageFromImage(nil)
	imgCanvas.FillMode = canvas.ImageFillContain
	imgCanvas.SetMinSize(fyne.NewSize(500, 700))
	scrollable := container.NewScroll(imgCanvas)

	textArea := widget.NewMultiLineEntry()
	textArea.Wrapping = fyne.TextWrapWord

	pageLabel := widget.NewLabel("")
	statusLabel := widget.NewLabel("")

	var (
		curPath  string
		curPage  int
		curTotal int
	)

	var render func(int)

	prevBtn := widget.NewButton("Prev", func() {
		if curPage > 0 {
			render(curPage - 1)
		}
	})
	nextBtn := widget.NewButton("Next", func() {
		if curPage < curTotal-1 {
			render(curPage + 1)
		}
	})
	ocrBtn := widget.NewButton("OCR Page", nil)
	openBtn := widget.NewButton("Open PDF", nil)

	prevBtn.Disable()
	nextBtn.Disable()
	ocrBtn.Disable()

	setBusy := func(busy bool) {
		if busy {
			openBtn.Disable()
			prevBtn.Disable()
			nextBtn.Disable()
			ocrBtn.Disable()
			return
		}
		openBtn.Enable()
		if curTotal == 0 {
			prevBtn.Disable()
			nextBtn.Disable()
			ocrBtn.Disable()
			return
		}
		ocrBtn.Enable()
		if curPage <= 0 {
			prevBtn.Disable()
		} else {
			prevBtn.Enable()
		}
		if curPage >= curTotal-1 {
			nextBtn.Disable()
		} else {
			nextBtn.Enable()
		}
	}

	updateLabel := func() {
		if curTotal == 0 {
			pageLabel.SetText("")
			return
		}
		pageLabel.SetText(fmt.Sprintf("Page %d of %d", curPage+1, curTotal))
	}

	render = func(page int) {
		path := curPath
		if path == "" {
			return
		}
		setBusy(true)

		go func() {
			resp, err := worker.request(map[string]any{
				"cmd":  "render",
				"path": path,
				"page": page,
				"dpi":  120,
			}, nil)
			if err != nil {
				fyne.Do(func() {
					dialog.ShowError(err, win)
					setBusy(false)
				})
				return
			}
			if t, _ := resp["type"].(string); t == "error" {
				fyne.Do(func() {
					dialog.ShowError(fmt.Errorf("%v", resp["message"]), win)
					setBusy(false)
				})
				return
			}
			pngB64, _ := resp["png_base64"].(string)
			pngBytes, err := base64.StdEncoding.DecodeString(pngB64)
			if err != nil {
				fyne.Do(func() {
					dialog.ShowError(err, win)
					setBusy(false)
				})
				return
			}
			img, _, err := image.Decode(bytes.NewReader(pngBytes))
			if err != nil {
				fyne.Do(func() {
					dialog.ShowError(err, win)
					setBusy(false)
				})
				return
			}
			pages := 0
			if p, ok := resp["pages"].(float64); ok {
				pages = int(p)
			}
			fyne.Do(func() {
				imgCanvas.Image = img
				imgCanvas.Refresh()
				curPage = page
				if pages > 0 {
					curTotal = pages
				}
				updateLabel()
				textArea.SetText("")
				setBusy(false)
			})
		}()
	}

	ocrBtn.OnTapped = func() {
		path := curPath
		page := curPage
		if path == "" {
			return
		}
		setBusy(true)
		textArea.SetText("")
		statusLabel.SetText("starting...")

		var lastStage string
		onProgress := func(ev map[string]any) {
			kind, _ := ev["kind"].(string)
			var text string
			switch kind {
			case "stage":
				if name, ok := ev["name"].(string); ok {
					lastStage = name
					text = "● " + name
				}
			case "tqdm":
				desc, _ := ev["desc"].(string)
				n, _ := ev["n"].(float64)
				total, _ := ev["total"].(float64)
				prefix := lastStage
				if prefix == "" {
					prefix = "..."
				}
				if total > 0 {
					text = fmt.Sprintf("● %s — %s %d/%d", prefix, desc, int(n), int(total))
				} else if desc != "" {
					text = fmt.Sprintf("● %s — %s", prefix, desc)
				} else {
					text = "● " + prefix
				}
			default:
				return
			}
			fyne.Do(func() { statusLabel.SetText(text) })
		}

		go func() {
			resp, err := worker.request(map[string]any{
				"cmd":  "ocr",
				"path": path,
				"page": page,
			}, onProgress)
			if err != nil {
				fyne.Do(func() {
					dialog.ShowError(err, win)
					textArea.SetText("")
					statusLabel.SetText("")
					setBusy(false)
				})
				return
			}
			if t, _ := resp["type"].(string); t == "error" {
				fyne.Do(func() {
					dialog.ShowError(fmt.Errorf("%v", resp["message"]), win)
					textArea.SetText("")
					statusLabel.SetText("")
					setBusy(false)
				})
				return
			}
			text, _ := resp["text"].(string)
			fyne.Do(func() {
				textArea.SetText(text)
				statusLabel.SetText("done")
				setBusy(false)
			})
		}()
	}

	openBtn.OnTapped = func() {
		path, err := nfd.File().Title("Open PDF").Filter("PDF files (*.pdf)", "pdf").Load()
		if err == nfd.ErrCancelled {
			return
		}
		if err != nil {
			dialog.ShowError(err, win)
			return
		}
		curPath = path
		curPage = 0
		curTotal = 0
		textArea.SetText("")
		render(0)
	}

	leftBox := container.NewHBox(openBtn, ocrBtn)
	navBox := container.NewHBox(prevBtn, pageLabel, nextBtn)
	topBar := container.NewBorder(nil, nil, leftBox, navBox, nil)

	split := container.NewHSplit(scrollable, textArea)
	split.Offset = 0.6

	cpuBar := newVBar("CPU", color.NRGBA{R: 80, G: 170, B: 220, A: 255})
	ramBar := newVBar("RAM", color.NRGBA{R: 110, G: 200, B: 130, A: 255})
	gpuBar := newVBar("GPU", color.NRGBA{R: 220, G: 140, B: 80, A: 255})
	vramBar := newVBar("VRAM", color.NRGBA{R: 200, G: 110, B: 200, A: 255})
	tempBar := newVBar("TEMP", color.NRGBA{R: 220, G: 90, B: 90, A: 255})
	metricsCol := container.NewGridWithColumns(5, cpuBar, ramBar, gpuBar, vramBar, tempBar)

	stop := make(chan struct{})
	defer close(stop)
	go runMetricsLoop(stop, cpuBar, ramBar, gpuBar, vramBar, tempBar)

	win.SetContent(container.NewBorder(topBar, statusLabel, metricsCol, nil, split))
	win.Resize(fyne.NewSize(1200, 800))
	win.CenterOnScreen()
	win.ShowAndRun()
}
