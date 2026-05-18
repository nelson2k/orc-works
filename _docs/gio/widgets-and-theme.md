# Widgets and Theme

Packages: [gioui.org/widget](../../repos-folder/gio/widget/), [gioui.org/widget/material](../../repos-folder/gio/widget/material/)

The `widget` package holds *state and event handling* for interactive controls. It deliberately does not render anything — drawing is a separate concern, delegated to a theme. The `widget/material` package is the official Material-design theme. Custom themes are just structs with their own rendering style methods.

## State widgets (`widget/`)

| Type | Purpose | Key API |
|---|---|---|
| `widget.Clickable` | Button-like state | `.Clicked(gtx)`, `.Hovered()`, `.Pressed()`, `.Layout(gtx, w)` |
| `widget.Bool` | Toggle (checkbox / switch) | `.Value`, `.Update(gtx)`, `.Layout(gtx, ...)` |
| `widget.Enum` | Radio-group selector | `.Value` string, `.Update(gtx)`, `.Layout(gtx, key, ...)` |
| `widget.Float` | 0..1 float (slider, etc.) | `.Value`, `.Update(gtx)`, `.Layout(gtx, ...)` |
| `widget.Editor` | Single- or multi-line text input | `.SetText(s)`, `.Text()`, `.SetCaret(start, end)`, `.Events(gtx)`, `.Layout(gtx, shaper, font, size, fn)` |
| `widget.Selectable` | Read-only text with selection / copy | `.SetText(s)`, `.Selection()`, `.Layout(gtx, shaper, font, size, fn)` |
| `widget.List` | Stateful list (wraps `layout.List`) with built-in scrollbar | `.Axis`, `.Layout(gtx, n, fn)` |
| `widget.Scrollbar` | Standalone scrollbar | `.Layout(gtx, axis, viewport, content)` |
| `widget.Icon` | Vector icon resource | `widget.NewIcon(bytes)`, `.Layout(gtx, color)` |
| `widget.Image` | Image holder with fit/position | `.Layout(gtx)` |
| `widget.Border` | Border around a sub-widget | `.Layout(gtx, w)` |
| `widget.Decorations` | Window-decoration state (custom titlebars) | |

### `widget.Clickable`

```go
var btn widget.Clickable
// ... in frame ...
for btn.Clicked(gtx) { handleClick() }
return btn.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
    return paint.Fill(gtx.Ops, color.NRGBA{...}), layout.Dimensions{Size: ...}
})
```

The `Clickable.Layout(gtx, w)` wraps a sub-widget with hit-testing for pointer events. It also exposes:

- `Clicks()` — slice of past click events
- `History()` — recent press positions
- `Hovered() bool`
- `Pressed() bool`
- `Focus()`, `Focused() bool`

### `widget.Editor`

```go
var ed widget.Editor
ed.SingleLine = true
ed.Submit = true   // Enter produces SubmitEvent

for _, e := range ed.Events() {
    switch e := e.(type) {
    case widget.ChangeEvent: ...
    case widget.SubmitEvent: ...
    case widget.SelectEvent: ...
    }
}

ed.Layout(gtx, shaper, font, textSize, func(gtx layout.Context) layout.Dimensions {
    return ... // optional decoration (hint, suffix)
})
```

The editor handles IME composition, clipboard, drag-select, undo/redo via the IME protocol.

## Theme

```go
type Theme struct {
    Shaper   *text.Shaper
    Palette  // Bg, Fg, ContrastBg, ContrastFg
    TextSize unit.Sp
    Icon     struct {
        CheckBoxChecked, CheckBoxUnchecked, RadioChecked, RadioUnchecked *widget.Icon
    }
    Face       font.Typeface
    FingerSize unit.Dp   // minimum tap target size
}

th := material.NewTheme()
th.Palette.ContrastBg = color.NRGBA{R: 80, G: 120, B: 220, A: 255}
```

Each top-level window normally gets its own `*Theme` (because each window should have its own `*text.Shaper`).

## Material widgets

Material constructors take a `*Theme` and a widget state, return a `Style` struct whose `Layout(gtx)` actually renders.

```go
material.Button(th, &btn, "Save").Layout(gtx)
material.IconButton(th, &btn, icon, "save").Layout(gtx)
material.ButtonLayout(th, &btn).Layout(gtx, ...inner)

material.CheckBox(th, &checked, "Remember me").Layout(gtx)
material.Switch(th, &enabled, "Wi-Fi").Layout(gtx)
material.RadioButton(th, &group, "key", "Label").Layout(gtx)

material.Editor(th, &ed, "Hint text").Layout(gtx)

material.Slider(th, &volume).Layout(gtx)
material.ProgressBar(th, 0.75).Layout(gtx)
material.ProgressCircle(th, 0.75).Layout(gtx)
material.Loader(th).Layout(gtx)

material.H1(th, "Heading").Layout(gtx)   // also H2..H6, Subtitle1/2, Body1/2, Caption, Overline
material.Label(th, unit.Sp(14), "text").Layout(gtx)

material.Decorations(th, &deco, actions, "Title").Layout(gtx)

material.List(th, &list).Layout(gtx, n, fn)
material.Scrollbar(th, &state).Layout(gtx, axis, viewport, contentLength)
```

Each material constructor returns a Style struct (e.g. `ButtonStyle`) with exported fields you can override (text color, background, padding) before calling `.Layout`.

## Custom widgets

A custom widget is just a function: `func(gtx layout.Context) layout.Dimensions`. The recipe:

1. Hold any persistent state in your own struct.
2. Inside `Layout`, draw via `clip` + `paint`, register input handlers via `pointer.InputOp` / `key.FocusOp` / `gesture.*`.
3. Return the size you consumed.

A custom *theme* is just a struct holding styling fields (colors, sizes, shapers) plus constructor functions that return your own `Style` types. There's nothing magical about `material` — it's a sample implementation.

## Icons

```go
import "golang.org/x/exp/shiny/materialdesign/icons"

icon, err := widget.NewIcon(icons.ActionHome)
material.IconButton(th, &btn, icon, "home").Layout(gtx)
```

The shiny package ships the full Material icon set as `[]byte` constants.
