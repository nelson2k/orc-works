# Events

Three event flavors flow through a Wails v3 app:

1. **Application events** — OS-fired things (theme changed, app started, app activated).
2. **Window events** — per-window things (focus, blur, closing, resize).
3. **Custom events** — your own pub/sub between Go and JS.

All three use the same `app.Event` API.

## Application events

From [`v3/examples/events/main.go`](../../repos-folder/wails/v3/examples/events/main.go):

```go
app.Event.OnApplicationEvent(events.Common.ApplicationStarted, func(event *application.ApplicationEvent) {
    app.Logger.Info("events.Common.ApplicationStarted fired!")
})

app.Event.OnApplicationEvent(events.Common.ThemeChanged, func(event *application.ApplicationEvent) {
    if event.Context().IsDarkMode() {
        app.Logger.Info("System is now using dark mode!")
    }
})
```

Event identifiers are typed Go constants under `events.Common.*` (platform-agnostic) and `events.Mac.*` / `events.Windows.*` / `events.Linux.*` (platform-specific). Compile-time safety.

## Window events and hooks

Windows have two reception modes:

- **`OnWindowEvent`** — listener; fires after the event happens.
- **`RegisterHook`** — interceptor; can cancel the event.

```go
win1.RegisterHook(events.Common.WindowClosing, func(e *application.WindowEvent) {
    countdown--
    if countdown == 0 {
        app.Logger.Info("Window 1 Closing!")
        return
    }
    e.Cancel()    // prevents the close
})

win2.OnWindowEvent(events.Common.WindowFocus, func(e *application.WindowEvent) {
    app.Logger.Info("Window 2 focused")
})
```

Calling `e.Cancel()` on a hook stops the event from taking effect — that's how you can implement "are you sure you want to quit?" prompts.

## Custom events (Go ↔ JS)

```go
// Go side — listen
app.Event.On("myevent", func(e *application.CustomEvent) {
    app.Logger.Info("[Go] CustomEvent received", "name", e.Name, "data", e.Data, "sender", e.Sender)
})

// Go side — emit
app.Event.Emit("myevent", "hello")

// Per-window emit (more efficient than broadcast)
win2.EmitEvent("windowevent", "ooooh!")
```

```js
// JS side
import { Events } from "/wails/runtime.js";

Events.On("myevent", (event) => {
    console.log("got event", event.data);
});

Events.Emit({ name: "from-js", data: { x: 1 } });
```

Events also support cancellation — `e.IsCancelled()` lets one listener short-circuit later ones.

## Sender

When JS emits an event, the `sender` field carries the originating window name. When Go emits it from the application (not a window), the `sender` is blank. That lets a Go listener know whether the event came from "anywhere" or from a specific window.
