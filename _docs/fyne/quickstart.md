# Quickstart

The simplest possible Fyne app:

```go
package main

import (
    "fyne.io/fyne/v2/app"
    "fyne.io/fyne/v2/container"
    "fyne.io/fyne/v2/widget"
)

func main() {
    a := app.New()
    w := a.NewWindow("Hello")

    hello := widget.NewLabel("Hello Fyne!")
    w.SetContent(container.NewVBox(
        hello,
        widget.NewButton("Hi!", func() {
            hello.SetText("Welcome :)")
        }),
    ))

    w.ShowAndRun()
}
```

Run it:

```
go run main.go
```

`ShowAndRun()` is shorthand for `w.Show(); a.Run()` — both block until the master window closes or `a.Quit()` is called.

## The shape of every Fyne program

```
app.New() or app.NewWithID("com.example.mything")   // build the App
a.NewWindow("title")                                // build a Window
window.SetContent(<a CanvasObject>)                 // set the root content
window.ShowAndRun()                                 // show and block
```

The content is always a single `CanvasObject` — to put multiple things on screen you wrap them in a `container.Container` with a `Layout`, or in a widget like `Form` or `List` that owns its children.

## `app.New` vs `app.NewWithID`

```go
a := app.New()                          // simplest form
a := app.NewWithID("io.fyne.demo")      // required for Preferences API
```

The unique ID is used to scope per-app preferences (so two apps don't fight over the same key in the OS preferences store) and to wire up cloud sync. `FyneApp.toml` next to `main.go` sets the ID at build time so you can keep using bare `app.New()` and still get a stable ID — the metadata is then available via `a.Metadata()`.

## A typical wider example

```go
package main

import (
    "fyne.io/fyne/v2"
    "fyne.io/fyne/v2/app"
    "fyne.io/fyne/v2/container"
    "fyne.io/fyne/v2/data/binding"
    "fyne.io/fyne/v2/widget"
)

func main() {
    a := app.NewWithID("com.example.counter")
    w := a.NewWindow("Counter")
    w.Resize(fyne.NewSize(300, 200))

    count := binding.NewInt()
    label := widget.NewLabelWithData(binding.IntToString(count))

    incr := widget.NewButton("+", func() {
        v, _ := count.Get()
        count.Set(v + 1)
    })
    decr := widget.NewButton("-", func() {
        v, _ := count.Get()
        count.Set(v - 1)
    })

    w.SetContent(container.NewVBox(
        label,
        container.NewHBox(decr, incr),
    ))
    w.ShowAndRun()
}
```

`binding.NewInt` returns an `Int` data binding; `widget.NewLabelWithData` registers itself as a listener and re-renders automatically when the value changes. See the data-binding notes for the full pattern.

## Background work and the UI thread

All UI mutations must happen on the main goroutine. From a worker goroutine, use `fyne.Do(...)` (or the older `fyne.DoAndWait` variant) to marshal the call back:

```go
go func() {
    result := slowComputation()
    fyne.Do(func() {
        label.SetText(result)
    })
}()
```

The driver implements this via `DoFromGoroutine(fn, wait)` (see [driver.go](../../repos-folder/fyne/driver.go)).

## Lifecycle hooks

```go
a.Lifecycle().SetOnStarted(func() { /* main loop running */ })
a.Lifecycle().SetOnStopped(func() { /* app exiting */ })
a.Lifecycle().SetOnEnteredForeground(func() { /* gained focus */ })
a.Lifecycle().SetOnExitedForeground(func() { /* lost focus */ })
```

Particularly useful on mobile, where the OS suspends/resumes apps freely.

## Common imports

```go
import (
    "fyne.io/fyne/v2"                    // core interfaces, Size, Position, KeyEvent, ...
    "fyne.io/fyne/v2/app"                // app.New / NewWithID
    "fyne.io/fyne/v2/canvas"             // canvas primitives (Text, Image, Rectangle, Circle)
    "fyne.io/fyne/v2/container"          // VBox, HBox, Tabs, Split, Scroll, ...
    "fyne.io/fyne/v2/data/binding"       // reactive bindings
    "fyne.io/fyne/v2/data/validation"    // form validators
    "fyne.io/fyne/v2/dialog"             // stock dialogs
    "fyne.io/fyne/v2/driver/desktop"     // desktop-only extensions (cursors, modifier keys)
    "fyne.io/fyne/v2/layout"             // pre-built layouts
    "fyne.io/fyne/v2/storage"            // URI / file abstraction
    "fyne.io/fyne/v2/theme"              // colors, sizes, icons, fonts
    "fyne.io/fyne/v2/widget"             // stock widgets
)
```
