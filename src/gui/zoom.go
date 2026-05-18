package main

import (
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/canvas"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/driver/desktop"
	"fyne.io/fyne/v2/widget"
)

const (
	zoomStep    = 1.1
	zoomMinSide = float32(100)
	zoomMaxSide = float32(8000)
)

type zoomableImage struct {
	widget.BaseWidget
	img       *canvas.Image
	parent    *container.Scroll
	ctrlDown  *bool
	spaceDown *bool
}

func newZoomableImage(img *canvas.Image, ctrlDown, spaceDown *bool) *zoomableImage {
	z := &zoomableImage{img: img, ctrlDown: ctrlDown, spaceDown: spaceDown}
	z.ExtendBaseWidget(z)
	return z
}

func (z *zoomableImage) Dragged(e *fyne.DragEvent) {
	if z.parent == nil {
		return
	}
	if z.spaceDown == nil || !*z.spaceDown {
		return
	}
	off := z.parent.Offset
	off.X -= e.Dragged.DX
	off.Y -= e.Dragged.DY
	z.parent.Offset = off
	z.parent.Refresh()
}

func (z *zoomableImage) DragEnd() {}

func (z *zoomableImage) Cursor() desktop.Cursor {
	if z.spaceDown != nil && *z.spaceDown {
		return desktop.PointerCursor
	}
	return desktop.DefaultCursor
}

func (z *zoomableImage) Scrolled(e *fyne.ScrollEvent) {
	if z.ctrlDown != nil && *z.ctrlDown && e.Scrolled.DY != 0 {
		cur := z.img.MinSize()
		f := float32(zoomStep)
		if e.Scrolled.DY < 0 {
			f = 1.0 / zoomStep
		}
		nw := cur.Width * f
		nh := cur.Height * f
		if nw < zoomMinSide || nh < zoomMinSide {
			return
		}
		if nw > zoomMaxSide || nh > zoomMaxSide {
			return
		}
		z.img.SetMinSize(fyne.NewSize(nw, nh))
		z.img.Refresh()
		z.Refresh()
		if z.parent != nil {
			z.parent.Refresh()
		}
		return
	}
	if z.parent != nil {
		z.parent.Scrolled(e)
	}
}

func (z *zoomableImage) CreateRenderer() fyne.WidgetRenderer {
	return &zoomImageRenderer{z: z}
}

type zoomImageRenderer struct {
	z *zoomableImage
}

func (r *zoomImageRenderer) Layout(size fyne.Size) {
	r.z.img.Resize(size)
	r.z.img.Move(fyne.NewPos(0, 0))
}

func (r *zoomImageRenderer) MinSize() fyne.Size {
	return r.z.img.MinSize()
}

func (r *zoomImageRenderer) Objects() []fyne.CanvasObject {
	return []fyne.CanvasObject{r.z.img}
}

func (r *zoomImageRenderer) Refresh() {
	r.z.img.Refresh()
}

func (r *zoomImageRenderer) Destroy() {}
