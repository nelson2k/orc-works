# Layouts

A `fyne.Layout` (in [layout.go](../../repos-folder/fyne/layout.go)) decides where the children of a `*fyne.Container` go.

```go
type Layout interface {
    Layout([]CanvasObject, Size)   // assign Move / Resize to each child
    MinSize(objects []CanvasObject) Size
}
```

Bundled layouts live in [layout/](../../repos-folder/fyne/layout/). Each has both a constructor on the `layout` package and a convenience constructor on the `container` package (the latter creates the container in one call).

## Built-in layouts

| Layout | Constructor | Behavior |
|---|---|---|
| Box (vertical / horizontal) | `layout.NewVBoxLayout()` / `layout.NewHBoxLayout()` and `container.NewVBox(...)` / `container.NewHBox(...)` | Stack children with theme padding between. Children get their `MinSize`; the unused axis is full. |
| Grid | `layout.NewGridLayout(cols)` and `container.NewGridWithColumns(n, ...)` / `container.NewGridWithRows(n, ...)` | Equal-sized cells laid out in N columns (or rows). |
| GridWrap | `layout.NewGridWrapLayout(cellSize)` and `container.NewGridWrap(cellSize, ...)` | Cells of a fixed size, wrapping to new rows as needed. |
| Border | `layout.NewBorderLayout(top, bottom, left, right)` and `container.NewBorder(top, bottom, left, right, center...)` | Four edges plus a center that takes the remaining space. |
| Form | `layout.NewFormLayout()` | Two-column label + field layout; alternating children become labels and fields. Used internally by `widget.Form`. |
| Center | `layout.NewCenterLayout()` and `container.NewCenter(obj)` | Center the child at its `MinSize` inside the parent. |
| Stack (formerly Max) | `layout.NewStackLayout()` (or the deprecated `NewMaxLayout`) and `container.NewStack(...)` / `container.NewMax(...)` | Place all children at full size — they overlap. Useful for backgrounds. |
| Padded | `layout.NewPaddedLayout()` and `container.NewPadded(obj)` | Single child with theme padding on all sides. |
| Custom padded | `layout.NewCustomPaddedLayout(top, bottom, left, right)` | Same idea with explicit padding values. |
| Row-wrap | `layout.NewRowWrapLayout()` | Lays children out horizontally, wrapping to a new row when full. |
| Spacer | `layout.NewSpacer()` | A zero-min-size flexible filler; commonly used in HBox/VBox to push siblings to the edges. |

## Writing a custom layout

```go
type CustomLayout struct{}

func (c *CustomLayout) MinSize(objects []fyne.CanvasObject) fyne.Size {
    w, h := float32(0), float32(0)
    for _, o := range objects {
        sz := o.MinSize()
        if sz.Width > w { w = sz.Width }
        h += sz.Height
    }
    return fyne.NewSize(w, h)
}

func (c *CustomLayout) Layout(objects []fyne.CanvasObject, containerSize fyne.Size) {
    y := float32(0)
    for _, o := range objects {
        sz := o.MinSize()
        o.Move(fyne.NewPos(0, y))
        o.Resize(fyne.NewSize(containerSize.Width, sz.Height))
        y += sz.Height
    }
}

// Then:
c := container.New(&CustomLayout{}, obj1, obj2, obj3)
```

Two things the layout API does not provide built-in — both have to be done manually if you need them:

- **Constraints / proportions.** Layouts pass each child its full or min size; there's no built-in "this child should take 30% of the width". You can express it in a custom layout, or use a `container.Split` for two-pane proportional sizing.
- **Z-order.** Order of children in `Objects` determines paint order — later objects draw on top. Use `Stack` to compose layers explicitly.

## Padding and theme

The padding inserted between siblings by `VBox` / `HBox` / `Padded` / etc. comes from `theme.Padding()` at render time. It changes with theme and respects user scaling. Inset values you read from a layout snapshot are stable for that snapshot; query the theme again if the user changes it.

## Containers that aren't `*fyne.Container`

Some widgets in [container/](../../repos-folder/fyne/container/) are widget-based composite containers, not bare `Container` + `Layout`:

- `container.AppTabs` / `container.DocTabs` — tab control with tab strip.
- `container.Split` — draggable resizer between two panes.
- `container.Scroll` — scrollable viewport.
- `container.InnerWindow` — title-barred draggable inner window.
- `container.MultipleWindows` — manages overlapping inner windows.

These return `*container.AppTabs` etc. (concrete types implementing `fyne.CanvasObject` via embedded `widget.BaseWidget`), not `*fyne.Container`.

## Padding-less / fullscreen layouts

`window.SetPadded(false)` strips the outer window padding. Inside, `container.NewWithoutLayout(...)` plus manual `Move`/`Resize` gives you raw pixel control — used for splash screens and games.
