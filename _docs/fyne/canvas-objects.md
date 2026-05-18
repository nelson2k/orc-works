# Canvas objects, widgets, behaviors

Everything renderable in Fyne implements `fyne.CanvasObject` (in [canvasobject.go](../../repos-folder/fyne/canvasobject.go)). The two specializations are **canvas primitives** (in `canvas/`) and **widgets** (in `widget/`).

## `CanvasObject`

Minimum contract:

```go
type CanvasObject interface {
    MinSize() Size
    Move(Position)
    Position() Position
    Resize(Size)
    Size() Size

    Hide()
    Visible() bool
    Show()

    Refresh()
}
```

Concrete objects live in either `canvas/` (drawing primitives) or `widget/` (interactive widgets). Containers (`*fyne.Container`) and `WidgetRenderer` arrange children.

### Move/Resize and layouts

`Move` and `Resize` should not be called directly on an object inside a container with a layout — the layout manages those. For raw positioning, use `container.NewWithoutLayout(...)`.

`MinSize()` is the contract that lets layouts negotiate space. Most widgets compute it from their content (e.g. label measures its text). Most layouts iterate the children to compute the container's min size, then assign positions and sizes in `Layout(objs, total)`.

## Canvas primitives ([canvas/](../../repos-folder/fyne/canvas/))

Implement `CanvasObject` directly; rendered by the driver without going through a `WidgetRenderer`.

| Type | Purpose |
|---|---|
| `canvas.Text` | A line of text with font, style, size, color, alignment |
| `canvas.Image` | A bitmap or vector image with fill mode |
| `canvas.Rectangle` | A filled or stroked rectangle |
| `canvas.Circle` | A filled or stroked circle |
| `canvas.Line` | A line segment |
| `canvas.Raster` | A function `(w,h) -> image.Image` invoked each frame — for procedural rendering |
| `canvas.LinearGradient` / `canvas.RadialGradient` | Gradient fills |
| `canvas.Arc` | Arc segment (2.7+) |
| `canvas.BezierCurve` | Quadratic / cubic Bezier (2.7+) |
| `canvas.ArbitraryPolygon` | Arbitrary closed polygon (2.7+) |
| `canvas.RegularPolygon` | Regular polygon (2.7+) |
| `canvas.Square` | Forced-square rectangle (2.7+) |
| `canvas.Blur` | Blur filter wrapping a child (2.7+) |
| `canvas.Animation` | Time-based interpolator |

`canvas.Refresh(obj)` is the low-level "tell the driver this object's appearance changed" call. Most code calls `obj.Refresh()` instead.

## Widgets ([widget/](../../repos-folder/fyne/widget/))

Implement `fyne.Widget` which extends `CanvasObject` with a `CreateRenderer() WidgetRenderer` method. The renderer owns the actual rendered objects and is the only thing the driver sees.

The widget package ships a large catalog. Common ones:

| Widget | Purpose |
|---|---|
| `widget.Label` / `widget.NewLabel`, `widget.NewLabelWithStyle`, `widget.NewLabelWithData` | Static text |
| `widget.Button` / `widget.NewButton`, `widget.NewButtonWithIcon` | Text or icon button |
| `widget.Entry` / `widget.NewEntry`, `NewMultiLineEntry`, `NewPasswordEntry`, `widget.DateEntry` | Single/multi-line text input |
| `widget.SelectEntry` | Entry with a dropdown suggestion list |
| `widget.Select` | Dropdown picker |
| `widget.Check` / `widget.CheckGroup` | Checkboxes |
| `widget.RadioGroup` | Radio buttons |
| `widget.Slider` | Range slider |
| `widget.ProgressBar` / `widget.ProgressBarInfinite` | Progress |
| `widget.Activity` | Spinner-style activity indicator (2.6+) |
| `widget.Form` | Layout of `FormItem`s with submit/cancel callbacks and validators |
| `widget.Hyperlink` | Tap to open URL |
| `widget.Icon` | Themed icon |
| `widget.Separator` | Horizontal/vertical rule |
| `widget.Card` | Title + subtitle + content card |
| `widget.RichText` | Multi-style text rendering with `RichTextSegment`s |
| `widget.Markdown` / `widget.NewRichTextFromMarkdown` | Markdown to RichText |
| `widget.Calendar` | Date picker (2.5+) |
| `widget.Toolbar` | Horizontal action bar |
| `widget.Menu` / `widget.PopUpMenu` | Context menus |
| `widget.Accordion` | Collapsible sections |
| `widget.List` | Virtualized list view with a data provider |
| `widget.Tree` | Hierarchical tree |
| `widget.Table` | Cell-based table with row/column virtualization |
| `widget.GridWrap` | Wrapping grid of items |
| `widget.TextGrid` | Monospace character grid (terminal-like) |
| `widget.FileIcon` | Themed icon for a `URI` |
| `widget.PopUp` | Floating popup attached to a canvas |
| `widget.Selectable` | Wrapper that makes any object selectable in a list |

### `BaseWidget`

Most widgets embed `widget.BaseWidget` which gives them the standard `CanvasObject` glue (size/position/visibility/refresh). To make your own widget:

```go
type MyWidget struct {
    widget.BaseWidget
    // fields
}

func NewMyWidget() *MyWidget {
    m := &MyWidget{}
    m.ExtendBaseWidget(m)   // wire BaseWidget to the concrete type
    return m
}

func (m *MyWidget) CreateRenderer() fyne.WidgetRenderer {
    // build the objects, return a WidgetRenderer
}
```

`ExtendBaseWidget` is required so `BaseWidget` knows the outermost type — otherwise refresh/min-size calls would bottom out at the base.

`WidgetRenderer`:

```go
type WidgetRenderer interface {
    Destroy()
    Layout(Size)
    MinSize() Size
    Objects() []CanvasObject
    Refresh()
}
```

`Objects()` is what the renderer draws. `Refresh()` should re-apply theme colors, re-read the widget's state, and request a redraw of each child.

## Behavior interfaces

Defined in [canvasobject.go](../../repos-folder/fyne/canvasobject.go). Any object can implement any subset; the runtime checks at event-dispatch time.

| Interface | When fyne calls it |
|---|---|
| `Tappable` | Single click / single tap |
| `SecondaryTappable` | Right-click / long-tap |
| `DoubleTappable` | Two clicks within `Driver.DoubleTapDelay()` |
| `Draggable` | Mouse drag with held button |
| `Focusable` | Gains/loses keyboard focus; receives `TypedRune` and `TypedKey` |
| `Scrollable` | Mouse-wheel / two-finger scroll |
| `Shortcutable` | Keyboard shortcuts (Cmd-C, Cmd-V, ...) |
| `Disableable` | Has `Enable` / `Disable` / `Disabled()` states |
| `Tabbable` | `AcceptsTab()` returns true → tab key delivered to `TypedKey` instead of changing focus |
| `Accessible` | Provides `AccessibilityLabel` / `AccessibilityRole` to screen readers (2.8+) |

## Container ([container/](../../repos-folder/fyne/container/))

`*fyne.Container` is a `CanvasObject` with a `Layout` and a list of child `CanvasObject`s. Construct via the `container.*` factory functions, not the deprecated `fyne.NewContainer*`:

```go
container.New(layout, objects...)
container.NewWithoutLayout(objects...)
container.NewVBox(objs...)
container.NewHBox(objs...)
container.NewGridWithColumns(n, objs...)
container.NewGridWithRows(n, objs...)
container.NewBorder(top, bottom, left, right, center)
container.NewCenter(obj)
container.NewMax(objs...)        // stack on top of each other
container.NewPadded(obj)
container.NewScroll(obj)
container.NewHScroll(obj) / NewVScroll(obj)
container.NewAppTabs(...) / container.NewDocTabs(...)
container.NewHSplit(left, right) / NewVSplit(top, bottom)
container.NewInnerWindow(title, content)   // SDI inside main window (2.4+)
container.NewMultipleWindows(...)          // overlapping inner windows (2.4+)
container.NewStack(objs...)                // z-stack
```

## Drawing order

Children are drawn in the order they appear in `Objects()`. Later objects are on top. `Move(Position{})` is the upper-left corner; coordinates are in fyne's "logical pixel" units, scaled by `Canvas.Scale()` for the physical display.
