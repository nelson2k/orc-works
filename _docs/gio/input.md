# Input

Packages under [gioui.org/io/](../../repos-folder/gio/io/): `event`, `pointer`, `key`, `clipboard`, `transfer`, `semantic`, `system`. Plus [gioui.org/gesture](../../repos-folder/gio/gesture/) for higher-level recognizers.

Gio routes input by *tag*. You register a handler op with a tag (a Go pointer or comparable value), and during the frame you ask the queue for events filtered by that tag.

## The model

```
              ┌─────────────────────────────────────────┐
   widget ─►  │ register Filter for tag T               │  via input.Op / pointer.InputOp / key.FocusOp
              └─────────────────────────────────────────┘
              ┌─────────────────────────────────────────┐
              │ Frame ends → input router dispatches    │
              └─────────────────────────────────────────┘
              ┌─────────────────────────────────────────┐
   widget ─►  │ for {                                   │  next frame
              │   e, ok := gtx.Source.Event(filter)     │
              │   if !ok { break }                      │
              │   handle(e)                             │
              │ }                                       │
              └─────────────────────────────────────────┘
```

`gtx.Source` is an `input.Source`. `Source.Event(filters ...event.Filter)` yields the next event matching any of the filters.

## `io/event`

```go
type Event interface{ ImplementsEvent() }
type Tag    any           // identity used for routing
type Filter interface{ ImplementsFilter() }
```

Every event type implements `Event`; every filter type implements `Filter`. You ask the source for events that pass any of a set of filters.

`event.Op(o *op.Ops, tag Tag)` claims the tag at the current clip; subsequent events that land inside the clip target this tag.

## Pointer events

[io/pointer](../../repos-folder/gio/io/pointer/)

```go
type Event struct {
    Kind      Kind        // Press, Release, Move, Drag, Cancel, Enter, Leave, Scroll
    Source    Source      // Mouse, Touch, Stylus
    PointerID ID
    Priority  Priority
    Time      time.Duration
    Buttons   Buttons     // ButtonPrimary, ButtonSecondary, ButtonTertiary
    Position  f32.Point
    Scroll    f32.Point
    Modifiers key.Modifiers
}

type Filter struct {
    Target    Tag
    Kinds     Kind
    ScrollX   ScrollRange
    ScrollY   ScrollRange
}
```

To declare a clickable area:

```go
defer pointer.PassOp{}.Push(ops).Pop()      // optional, let events pass through
rect := clip.Rect(image.Rectangle{Max: size}).Push(ops)
event.Op(ops, tag)
pointer.CursorPointer.Add(ops)              // request a cursor
rect.Pop()

// ... next frame:
for {
    e, ok := gtx.Source.Event(pointer.Filter{Target: tag, Kinds: pointer.Press | pointer.Release})
    if !ok { break }
    // handle e
}
```

`pointer.GrabCmd{Tag: t, ID: pid, Grab: true}` requests exclusive grab of a pointer (so drags continue even when the cursor leaves the widget).

### Cursor

`pointer.CursorPointer.Add(ops)` requests a cursor while the pointer is over the current clip. Variants: `CursorDefault`, `CursorText`, `CursorPointer`, `CursorCrosshair`, `CursorRowResize`, `CursorColResize`, ...

## Key events

[io/key](../../repos-folder/gio/io/key/)

```go
type Event struct {
    Name      Name
    Modifiers Modifiers
    State     State    // Press, Release
}
type EditEvent struct{ Range Range; Text string }
type FocusEvent struct{ Focus bool }

type Filter struct {
    Focus    Tag
    Required Modifiers
    Optional Modifiers
    Name     Name
}

type FocusFilter struct{ Target Tag }
```

A widget that wants the keyboard focus:

```go
event.Op(ops, tag)
key.FocusCmd{Tag: tag}.Add(ops)              // request focus
key.InputHintOp{Tag: tag, Hint: key.HintText}.Add(ops)  // IME hint
key.SoftKeyboardCmd{Show: true}.Add(ops)

// next frame:
for {
    e, ok := gtx.Source.Event(
        key.FocusFilter{Target: tag},
        key.Filter{Focus: tag, Name: "Enter"},
        key.Filter{Focus: tag, Name: key.NameTab, Required: key.ModShift},
    )
    if !ok { break }
    // ...
}
```

`Modifiers` constants: `ModCtrl`, `ModShift`, `ModAlt`, `ModSuper`, `ModCommand`. Platform-specific resolutions live in `mod_darwin.go` / `mod_js.go`.

`Name` is a stable key identifier — `"A"`, `"Enter"`, `key.NameEscape`, `key.NameLeftArrow`, etc.

## Selection / IME

Editors interact with the OS IME via:

- `key.SelectionCmd{Tag, Start, End}` — propagate selection
- `key.SnippetCmd{Tag, Range, Text}` — push a text snippet for composition
- `key.EditEvent` — incoming edits from the IME
- `key.SelectionEvent`, `key.SnippetEvent` — OS-driven changes

## Clipboard

[io/clipboard](../../repos-folder/gio/io/clipboard/)

```go
clipboard.WriteOp{Text: "hello"}.Add(ops)
clipboard.ReadOp{Tag: tag}.Add(ops)

// later:
for {
    e, ok := gtx.Source.Event(transfer.TargetFilter{Target: tag, Type: "application/text"})
    if !ok { break }
    // handle e
}
```

## Transfer (drag-and-drop)

[io/transfer](../../repos-folder/gio/io/transfer/)

A drag-source widget posts a `transfer.SourceFilter`, a drop-target widget posts a `transfer.TargetFilter`. The router negotiates the MIME type and delivers a `transfer.DataEvent` to the target.

## Semantic / accessibility

[io/semantic](../../repos-folder/gio/io/semantic/)

`semantic.DescriptionOp{Description: "Save"}.Add(ops)`, `semantic.LabelOp{Label: "Save", Selected: false}.Add(ops)`, `semantic.SelectedOp{}.Add(ops)`, `semantic.Button.Add(ops)`. The OS-side accessibility tree (VoiceOver, TalkBack, Windows Narrator) consumes these.

## Gestures

[gesture/](../../repos-folder/gio/gesture/) — recognized gestures built on raw pointer events:

```go
var click gesture.Click
var drag  gesture.Drag
var hover gesture.Hover
var scroll gesture.Scroll
```

In your widget's Layout:

```go
event.Op(ops, &click)
click.Add(ops)        // recognizer
// ...
for {
    e, ok := click.Update(gtx)
    if !ok { break }
    switch e.Kind {
    case gesture.KindClick:  ...
    case gesture.KindPress:  ...
    case gesture.KindCancel: ...
    }
}
```

`gesture.Click.Add(ops)` filters pointer events for press/release timing and reports a `ClickEvent` with kind (`KindClick`, `KindPress`, `KindRelease`, `KindCancel`) plus button info.

`gesture.Drag.Update(gtx)` produces drag deltas. `gesture.Scroll` produces wheel/two-finger pan deltas, configured with a `ScrollRange`. `gesture.Hover` reports enter/leave.

## System events

[io/system](../../repos-folder/gio/io/system/) defines `system.Action` constants (close, raise, ...) used with `Window.Perform(action)`, and `system.Locale` carried through `Context`.
