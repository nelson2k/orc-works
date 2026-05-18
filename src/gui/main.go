package main

import (
	"bufio"
	"bytes"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"image"
	_ "image/png"
	"io"
	"os/exec"
	"path/filepath"
	"sync"

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

func (w *Worker) request(req map[string]any) (map[string]any, error) {
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
	return resp, nil
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

func main() {
	a := app.New()
	win := a.NewWindow("OCR Works")

	worker := &Worker{}
	defer worker.shutdown()

	imgCanvas := canvas.NewImageFromImage(nil)
	imgCanvas.FillMode = canvas.ImageFillContain
	imgCanvas.SetMinSize(fyne.NewSize(600, 800))
	scrollable := container.NewScroll(imgCanvas)

	pageLabel := widget.NewLabel("")

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
	prevBtn.Disable()
	nextBtn.Disable()

	updateNav := func() {
		if curTotal == 0 {
			pageLabel.SetText("")
			prevBtn.Disable()
			nextBtn.Disable()
			return
		}
		pageLabel.SetText(fmt.Sprintf("Page %d of %d", curPage+1, curTotal))
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

	render = func(page int) {
		path := curPath
		if path == "" {
			return
		}
		prevBtn.Disable()
		nextBtn.Disable()

		go func() {
			resp, err := worker.request(map[string]any{
				"cmd":  "render",
				"path": path,
				"page": page,
				"dpi":  120,
			})
			if err != nil {
				fyne.Do(func() {
					dialog.ShowError(err, win)
					updateNav()
				})
				return
			}
			if t, _ := resp["type"].(string); t == "error" {
				fyne.Do(func() {
					dialog.ShowError(fmt.Errorf("%v", resp["message"]), win)
					updateNav()
				})
				return
			}
			pngB64, _ := resp["png_base64"].(string)
			pngBytes, err := base64.StdEncoding.DecodeString(pngB64)
			if err != nil {
				fyne.Do(func() {
					dialog.ShowError(err, win)
					updateNav()
				})
				return
			}
			img, _, err := image.Decode(bytes.NewReader(pngBytes))
			if err != nil {
				fyne.Do(func() {
					dialog.ShowError(err, win)
					updateNav()
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
				updateNav()
			})
		}()
	}

	openBtn := widget.NewButton("Open PDF", func() {
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
		render(0)
	})

	navBox := container.NewHBox(prevBtn, pageLabel, nextBtn)
	topBar := container.NewBorder(nil, nil, openBtn, navBox, nil)
	win.SetContent(container.NewBorder(topBar, nil, nil, nil, scrollable))
	win.Resize(fyne.NewSize(900, 800))
	win.CenterOnScreen()
	win.ShowAndRun()
}
