# Widget catalog

Constructors all return a `*XxxWidget` value. Most expose fluent setters (`Size`, `Flags`, `OnXxx`, ...) before the final `.Build()` is called by the surrounding layout. You can also store widgets in variables and reuse them.

## Containers / windows ([Window.go](../../repos-folder/giu/Window.go))

| Constructor | Notes |
|---|---|
| `giu.SingleWindow()` | Full-window ImGui window, no titlebar, fills the master window |
| `giu.SingleWindowWithMenuBar()` | Same + room for a menu bar |
| `giu.Window(title)` | A regular ImGui sub-window |

Window methods: `.IsOpen(*bool)`, `.Flags(WindowFlags)`, `.Size(w, h)`, `.Pos(x, y)`, `.Layout(widgets ...Widget)`, `.RegisterKeyboardShortcuts(...)`.

## Layout ([Widgets.go](../../repos-folder/giu/Widgets.go), [Layout.go](../../repos-folder/giu/Layout.go))

| Constructor | Notes |
|---|---|
| `giu.Layout{...}` | Plain slice — Layout is `[]Widget` |
| `giu.Row(widgets...)` | Horizontal — wraps `ImGui::SameLine` between items |
| `giu.Column(widgets...)` | Vertical (default flow) — same as a Layout, mostly for symmetry |
| `giu.SameLine()` | Force the next widget on the same line |
| `giu.Separator()` | Horizontal rule |
| `giu.Spacing()` / `giu.Dummy(w, h)` | Explicit spacers |
| `giu.Child()` | A child region (its own scroll context) — chain `.Size`, `.Border`, `.Flags`, `.Layout` |
| `giu.Splitter(direction, *delta)` | Drag-to-resize handle |
| `giu.SplitLayout(direction, *sashPos, w1, w2)` | Two panes with a draggable splitter |
| `giu.StackWidget(visibleIdx, layouts ...Widget)` | Show one of N layouts (CardStack) |
| `giu.Condition(cond, ifWidget, elseWidget)` | Conditional rendering |
| `giu.Custom(func())` | Drop down to raw ImGui calls |

## Text and input ([TextWidgets.go](../../repos-folder/giu/TextWidgets.go))

| Constructor | Purpose |
|---|---|
| `giu.Label(s)` / `giu.Labelf(fmt, args...)` | Static label |
| `giu.InputText(*string)` | Single-line input — `.Hint(s)`, `.Flags(InputTextFlags)`, `.OnChange(fn)` |
| `giu.InputTextMultiline(*string)` | Multi-line |
| `giu.InputInt(*int32)` / `giu.InputFloat(*float32)` | Numeric input — `.StepFast(...)`, `.Format("%.2f")` |
| `giu.BulletText(s)` / `giu.Bullet()` | Bulleted text |

## Clickable widgets ([ClickableWidgets.go](../../repos-folder/giu/ClickableWidgets.go))

| Constructor | Purpose |
|---|---|
| `giu.Button(label)` | Standard button — `.OnClick(fn)`, `.Size(w, h)`, `.Disabled(bool)` |
| `giu.SmallButton(id)` | Compact button |
| `giu.ArrowButton(dir)` | Arrow (Up / Down / Left / Right) |
| `giu.InvisibleButton()` | Click target without chrome |
| `giu.ImageButton(*Texture)` / `giu.ImageButtonWithRgba(image.Image)` | Image-as-button |
| `giu.Checkbox(label, *bool)` | Checkbox |
| `giu.RadioButton(label, active bool)` | Radio — pair with `OnChange` |
| `giu.Selectable(label)` | A clickable list item |
| `giu.TreeNode(label)` | Expandable tree node — `.Layout(...)`, `.Flags(TreeNodeFlags)` |
| `giu.Link(text)` | Hyperlink — pairs with `pkg/browser` |

## Sliders and dragging ([SliderWidgets.go](../../repos-folder/giu/SliderWidgets.go))

| Constructor | Purpose |
|---|---|
| `giu.SliderInt(*int32, min, max)` | Horizontal int slider |
| `giu.VSliderInt(*int32, min, max)` | Vertical |
| `giu.SliderFloat(*float32, min, max)` | Float variant |
| `giu.DragInt(*int32)` / `giu.DragFloat(*float32)` | Drag-to-edit (no fixed range) |

All slider/drag types expose `.Size(w, h)`, `.Format("%.2f")`, `.Flags(SliderFlags)`.

## Images ([ImageWidgets.go](../../repos-folder/giu/ImageWidgets.go))

| Constructor | Purpose |
|---|---|
| `giu.Image(*Texture)` | Pre-uploaded texture |
| `giu.ImageWithRgba(image.Image)` | Upload-on-first-use |
| `giu.ImageWithFile(path)` | Lazy file load |
| `giu.ImageWithURL(url)` | Async fetch (uses [SurfaceLoaders.go](../../repos-folder/giu/SurfaceLoaders.go)) |

`.Size(w, h)`, `.OnClick(fn)`, `.Uv(...)`. The URL variant takes `.Timeout(d)`, `.LayoutForLoading(loadingWidget)`, `.LayoutForFailure(errWidget)`.

## Tables ([TableWidgets.go](../../repos-folder/giu/TableWidgets.go))

```go
giu.Table().
    Columns(
        giu.TableColumn("Name").Flags(TableColumnFlagsWidthFixed),
        giu.TableColumn("Score").Flags(TableColumnFlagsWidthStretch),
    ).
    Rows(
        giu.TableRow(giu.Label("Alice"), giu.Label("12")),
        giu.TableRow(giu.Label("Bob"),   giu.Label("23")),
    ).
    Flags(giu.TableFlagsResizable | giu.TableFlagsRowBg)
```

`giu.TreeTable()` / `giu.TreeTableRow(label, widgets...)` for hierarchical tables.

## Comboboxes / menus ([Widgets.go](../../repos-folder/giu/Widgets.go))

| Constructor | Purpose |
|---|---|
| `giu.Combo(label, preview, items, *selected)` | Standard combo box |
| `giu.ComboCustom(label, preview)` | Custom-rendered dropdown — `.Layout(widgets...)` |
| `giu.MainMenuBar()` | App-level menu bar (top of master window) |
| `giu.MenuBar()` | Window-level menu bar (inside a window) |
| `giu.Menu(label)` | Drop-down menu — `.Layout(menuItems...)` |
| `giu.MenuItem(label)` | Leaf menu item — `.OnClick(fn)`, `.Shortcut("Ctrl+S")`, `.Selected(*bool)` |
| `giu.ContextMenu()` | Right-click menu — `.Layout(...)` |

## Popups ([Popups.go](../../repos-folder/giu/Popups.go))

```go
giu.OpenPopup("my-popup")
giu.Popup("my-popup").Layout(
    giu.Label("Sure?"),
    giu.Row(
        giu.Button("OK").OnClick(func(){ giu.CloseCurrentPopup() }),
        giu.Button("Cancel").OnClick(func(){ giu.CloseCurrentPopup() }),
    ),
)
```

`giu.PopupModal(name)` is the modal variant.

## Message boxes ([Msgbox.go](../../repos-folder/giu/Msgbox.go))

```go
// once
layout := append(rootLayout, giu.PrepareMsgbox())

// triggered anywhere
giu.Msgbox("Saved", "Your file was saved").Buttons(giu.MsgboxButtonsOk).ResultCallback(...)
```

## Charts ([Plot.go](../../repos-folder/giu/Plot.go))

```go
giu.Plot("My chart").Plots(
    giu.Bar("series A", []float64{1, 2, 3}),
    giu.Line("series B", []float64{0.5, 1.5, 2.5}),
).Size(400, 200)
```

Variants: `Bar`, `BarH`, `Line`, `LineXY`, `PieChart`, `Scatter`, `ScatterXY`. Axis switch: `SwitchPlotAxes(x, y PlotXAxis|PlotYAxis)`.

## Other built-ins

| File | Provides |
|---|---|
| `Markdown.go` | `giu.Markdown(string)` — link clicks, code highlighting |
| `CodeEditor.go` | `giu.CodeEditor()` — syntax-highlighted editor |
| `MemoryEditor.go` | `giu.MemoryEditor()` — hex viewer |
| `Gizmo.go` | `giu.Gizmo(*ViewMatrix, *ProjectionMatrix)` + sub-gizmos (Grid, Cube, Manipulate, ViewManipulate) |
| `Canvas.go` | `giu.GetCanvas()` — low-level draw list (lines, paths, text, images) |
| `ProgressIndicator.go` | `giu.ProgressIndicator(label, w, h, radius)` |
| `ExtraWidgets.go` | `giu.DatePicker(id, *time.Time)`, `giu.ListBox([]string)`, `giu.RangeBuilder[T]`, `giu.MapRangeBuilder[T,U]` |
| `Tooltip` (in Widgets.go) | `giu.Tooltip(text)` |
| `TabBar` (in Widgets.go) | `giu.TabBar()` + `giu.TabItem(label).Layout(...)` |
| `ProgressBar` (in Widgets.go) | `giu.ProgressBar(fraction)` |
| `ColorEdit` (in Widgets.go) | `giu.ColorEdit(label, *color.RGBA)` |

## Builder helpers

```go
// virtualized list — recommended for >1000 items
giu.RangeBuilder("rows", items, func(i int, v Item) giu.Widget {
    return giu.Row(giu.Label(v.Name), giu.Button("X").OnClick(...))
})

giu.MapRangeBuilder("kv", myMap, func(k, v string) giu.Widget {
    return giu.Row(giu.Label(k), giu.Label(v))
})
```

For *extremely* long lists use `ListClipper`-backed widgets (see [ListClipper.go](../../repos-folder/giu/ListClipper.go)) — only visible rows pay rendering cost.
