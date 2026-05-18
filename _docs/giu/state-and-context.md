# State and context

[Context.go](../../repos-folder/giu/Context.go) defines the `*GIUContext` singleton (`giu.Context`) that holds per-application state: the cimgui-go backend, the font atlas, the input handler, the translator, the CSS stylesheet, texture queues, and a generic state map for widgets.

## Singleton

```go
var giu.Context *GIUContext
```

Set inside `NewMasterWindow`. Always one per program.

## ID generation

ImGui distinguishes widgets by stack-relative IDs. giu auto-generates them:

```go
id := giu.GenAutoID("button")   // returns "button##N" — N increments per frame
```

The library does this for you when widgets need an internal ID; user code rarely calls it directly. To pin an ID across frames (e.g. for state lookup), use the constructor variants that accept an explicit ID — `RangeBuilder("rows", items, fn)`, `DatePicker("dp1", &t)`, etc.

## Generic per-widget state

`SetState[T]` / `GetState[T]` store typed state attached to an ID. The type must implement `Disposable` (a `Dispose()` method called when state is cleaned up).

```go
type editorState struct {
    text string
}
func (*editorState) Dispose() {}

func myWidget() giu.Widget {
    s := giu.GetState[editorState](giu.Context, "myWidget")
    if s == nil {
        s = &editorState{}
        giu.SetState[editorState](giu.Context, "myWidget", s)
    }
    return giu.InputText(&s.text)
}
```

State is cleaned each frame for IDs that weren't accessed (via the dirty flag and `cleanStates()`). This is how giu reclaims state when widgets disappear without leaking.

## Pointer-binding state (the common case)

For simple types, just bind pointers into widgets:

```go
var name string
var ok   bool
var v    int32

giu.SingleWindow().Layout(
    giu.InputText(&name).Hint("Name"),
    giu.Checkbox("Agreed", &ok),
    giu.SliderInt(&v, 0, 100),
)
```

`InputText`, `InputTextMultiline`, `Checkbox`, `RadioButton` (pair with state), `Combo`, `ColorEdit`, the various `Slider*`, `Drag*`, and `InputInt/Float` all take pointers to the underlying value.

## OnChange callbacks

Most input widgets expose `.OnChange(func())` and/or `.OnClick(func())`. They're called after ImGui processes the event for the frame.

## Frame helpers

```go
giu.Update()                  // request a redraw (used after async state changes)
giu.GetMousePos()             // image.Point
giu.GetCursorScreenPos()      // image.Point — screen-relative
giu.GetCursorPos()            // image.Point — relative to current window
giu.SetCursorPos(pos)
giu.GetAvailableRegion()      // float32 w, h — remaining content region
giu.CalcTextSize(s)           // float32 w, h
giu.SetNextWindowSize(w, h)
giu.SetNextWindowPos(x, y)
giu.SetItemDefaultFocus()
giu.SetKeyboardFocusHere()
```

## Texture loading

Textures are uploaded on the main thread between frames.

```go
giu.NewTextureFromRgba(rgba, func(t *giu.Texture) {
    // texture ready — store it and use in giu.Image(t)
})

giu.EnqueueNewTextureFromRgba(rgba, cb)   // async enqueue from any goroutine
```

The context owns `textureLoadingQueue` and `textureFreeingQueue` (both `eapache/queue.v1` instances). The render loop drains them in `beforeRender`.

## Font atlas

`Context.FontAtlas` ([FontAtlasProsessor.go](../../repos-folder/giu/FontAtlasProsessor.go)) rebuilds the atlas incrementally when widgets contain glyphs that weren't yet included. This is how giu supports arbitrary scripts without explicit font registration.

```go
giu.Context.FontAtlas.AddFont("path/to/font.ttf", 16, ...)
giu.Context.FontAtlas.RegisterStringPure("草, español, العربية")  // pre-register glyph ranges
```

## Translation

```go
giu.Context.Translator = giu.NewBasicTranslator(/* table */)
```

`Translator` ([Translator.go](../../repos-folder/giu/Translator.go)) gates every label through `Translator.Translate(string) string`. `EmptyTranslator` is the default no-op.

## Input handler and shortcuts

[InputHandler.go](../../repos-folder/giu/InputHandler.go) holds the active `InputHandler`. Each frame it's run to dispatch registered shortcuts.

```go
wnd.RegisterKeyboardShortcuts(
    giu.WindowShortcut{Key: giu.KeyS, Modifier: giu.ModCtrl, Callback: onSave},
    giu.WindowShortcut{Key: giu.KeyQ, Modifier: giu.ModCtrl, Callback: onQuit},
)
```

Keys: see [Keycode.go](../../repos-folder/giu/Keycode.go). Modifiers: `ModCtrl`, `ModShift`, `ModAlt`, `ModSuper`, plus combinations.

## Events

[EventHandler.go](../../repos-folder/giu/EventHandler.go) exposes a fluent event tap useful for arbitrary widgets:

```go
giu.Button("Hover me").Build()

giu.Event().
    OnHover(func() { /* ... */ }).
    OnActive(func() { /* ... */ }).
    OnClick(giu.MouseButtonLeft, func() { /* ... */ }).
    Build()
```

`giu.Event()` reads ImGui's `IsItemHovered`, `IsItemActive`, etc. for the most recently submitted widget.

## Multi-threading

State mutation from non-render goroutines must funnel through `giu.Update()` so the next frame picks up changes. Texture loading from worker threads should use `EnqueueNewTextureFromRgba`. Drawing or ImGui calls outside the render goroutine are unsafe.
