package main

import (
	"image/color"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/canvas"
	"fyne.io/fyne/v2/theme"
	"fyne.io/fyne/v2/widget"
)

type vBar struct {
	widget.BaseWidget
	name     string
	value    float64
	valueStr string
	fillCol  color.Color
}

func newVBar(name string, fill color.Color) *vBar {
	b := &vBar{name: name, fillCol: fill, valueStr: "--"}
	b.ExtendBaseWidget(b)
	return b
}

func (b *vBar) set(frac float64, label string) {
	if frac < 0 {
		frac = 0
	}
	if frac > 1 {
		frac = 1
	}
	b.value = frac
	b.valueStr = label
	b.Refresh()
}

func (b *vBar) CreateRenderer() fyne.WidgetRenderer {
	track := canvas.NewRectangle(theme.Color(theme.ColorNameInputBackground))
	fill := canvas.NewRectangle(b.fillCol)
	value := canvas.NewText(b.valueStr, theme.Color(theme.ColorNameForeground))
	value.Alignment = fyne.TextAlignCenter
	value.TextSize = theme.CaptionTextSize()
	name := canvas.NewText(b.name, theme.Color(theme.ColorNameForeground))
	name.Alignment = fyne.TextAlignCenter
	name.TextSize = theme.CaptionTextSize()
	return &vBarRenderer{bar: b, track: track, fill: fill, value: value, name: name}
}

type vBarRenderer struct {
	bar         *vBar
	track, fill *canvas.Rectangle
	value, name *canvas.Text
}

func (r *vBarRenderer) Layout(size fyne.Size) {
	pad := float32(4)
	valueH := r.value.MinSize().Height
	nameH := r.name.MinSize().Height

	r.value.Move(fyne.NewPos(0, 0))
	r.value.Resize(fyne.NewSize(size.Width, valueH))

	nameY := size.Height - nameH
	r.name.Move(fyne.NewPos(0, nameY))
	r.name.Resize(fyne.NewSize(size.Width, nameH))

	trackTop := valueH + pad
	trackBottom := nameY - pad
	trackH := trackBottom - trackTop
	if trackH < 0 {
		trackH = 0
	}

	barW := size.Width - 8
	if barW < 6 {
		barW = 6
	}
	barX := (size.Width - barW) / 2

	r.track.Move(fyne.NewPos(barX, trackTop))
	r.track.Resize(fyne.NewSize(barW, trackH))

	fillH := trackH * float32(r.bar.value)
	r.fill.Move(fyne.NewPos(barX, trackTop+(trackH-fillH)))
	r.fill.Resize(fyne.NewSize(barW, fillH))
}

func (r *vBarRenderer) MinSize() fyne.Size {
	return fyne.NewSize(34, 120)
}

func (r *vBarRenderer) Refresh() {
	r.value.Text = r.bar.valueStr
	r.value.Color = theme.Color(theme.ColorNameForeground)
	r.value.Refresh()
	r.name.Color = theme.Color(theme.ColorNameForeground)
	r.name.Refresh()
	r.track.FillColor = theme.Color(theme.ColorNameInputBackground)
	r.track.Refresh()
	r.fill.FillColor = r.bar.fillCol
	r.fill.Refresh()
	r.Layout(r.bar.Size())
}

func (r *vBarRenderer) Objects() []fyne.CanvasObject {
	return []fyne.CanvasObject{r.track, r.fill, r.value, r.name}
}

func (r *vBarRenderer) Destroy() {}
