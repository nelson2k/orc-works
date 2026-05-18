# Data binding

[data/binding/](../../repos-folder/fyne/data/binding/). A reactive layer: data items that can be observed, mutated, and bound to widgets.

> All APIs in `data/binding` are safe to call from any goroutine — listeners are invoked on the UI thread.

## Base interfaces

```go
type DataItem interface {
    AddListener(DataListener)
    RemoveListener(DataListener)
}

type DataListener interface {
    DataChanged()
}
```

The listener's `DataChanged` is invoked when the data item changes. Listeners are also called once when first attached so they can pick up the current value.

`binding.NewDataListener(func() { ... })` wraps a closure as a listener.

## Scalar bindings

Per-type interfaces and constructors for the primitives Fyne supports:

| Type | Interface | Constructor | New(value) shorthand |
|---|---|---|---|
| bool | `binding.Bool` | `NewBool()` | `BindBool(*bool)` |
| int | `binding.Int` | `NewInt()` | `BindInt(*int)` |
| float64 | `binding.Float` | `NewFloat()` | `BindFloat(*float64)` |
| string | `binding.String` | `NewString()` | `BindString(*string)` |
| rune | `binding.Rune` | `NewRune()` | `BindRune(*rune)` |
| []byte | `binding.Bytes` | `NewBytes()` | `BindBytes(*[]byte)` |
| time.Duration | `binding.Duration` | `NewDuration()` | `BindDuration(*time.Duration)` |
| time.Time | `binding.Time` | `NewTime()` | `BindTime(*time.Time)` |
| *url.URL | `binding.URL` | `NewURL()` | `BindURL(**url.URL)` |
| `fyne.Resource` | `binding.Resource` | `NewResource()` | `BindResource(*fyne.Resource)` |

Each has `Get() (T, error)` / `Set(T) error`. The `Bind*` variants share storage with the passed pointer — useful for binding to a struct field.

## List, Tree, and Map bindings

`binding.UntypedList`, `binding.IntList`, `binding.StringList`, etc. — typed list bindings with `Append`, `Prepend`, `Set`, `GetValue(i)`, `Length`.

`binding.Map`, `binding.UntypedMap`, `binding.StringMap` — keyed bindings.

`binding.UntypedTree`, `binding.StringTree`, etc. — hierarchical bindings with parent/child relationships.

## Converters

Most widgets bind to a specific type. To bridge types, use the conversion helpers (in [data/binding/convert.go](../../repos-folder/fyne/data/binding/convert.go)):

```go
intToString := binding.IntToString(intBinding)
boolToString := binding.BoolToString(boolBinding)
floatToString := binding.FloatToString(floatBinding)
floatToStringFmt := binding.FloatToStringWithFormat(floatBinding, "%.2f")
timeToString := binding.TimeToString(timeBinding)
// also: IntToFloat, IntToString, StringToInt, StringToFloat, ...
```

Conversions are bi-directional — writing to the converted binding writes back through the original.

## Preferences-backed bindings ([data/binding/preference.go](../../repos-folder/fyne/data/binding/preference.go))

Bind directly to `App.Preferences()` keys. Useful for persistent settings:

```go
pref := a.Preferences()
volume := binding.BindPreferenceFloat("volume", pref)
slider := widget.NewSliderWithData(0, 100, volume)
```

Per type: `BindPreferenceBool`, `BindPreferenceInt`, `BindPreferenceFloat`, `BindPreferenceString`.

## Sprintf-based formatted strings ([data/binding/sprintf.go](../../repos-folder/fyne/data/binding/sprintf.go))

`binding.NewSprintf(format, items...)` is a read-only `String` binding that re-renders whenever any of its source bindings change:

```go
greeting := binding.NewSprintf("Hello, %s — your score is %d", nameBinding, scoreBinding)
label := widget.NewLabelWithData(greeting)
```

## Bind helpers in widgets

Most widgets have a `*WithData` constructor and / or a `.Bind(b)` method:

```go
widget.NewLabelWithData(stringBinding)
widget.NewEntryWithData(stringBinding)
widget.NewCheckWithData("Agree", boolBinding)
widget.NewProgressBarWithData(floatBinding)
widget.NewSliderWithData(0, 100, floatBinding)
widget.NewListWithData(listBinding, createItem, updateItem)
widget.NewTreeWithData(treeBinding, childUIDs, createNode, updateNode)
widget.NewTableWithData(...)
```

For `List` / `Tree` / `Table`, the data binding owns the model and the widget invokes the create/update callbacks for visible cells only — they scale to large datasets.

## Worker safety

You can call `Set` on a binding from any goroutine. The binding routes the listener invocations through the driver's `DoFromGoroutine` so widgets always update on the UI thread. This eliminates the explicit `fyne.Do(...)` boilerplate inside background workers that only need to update widget state.

## Custom data items

To make your own binding type, implement `DataItem` + your domain methods, and call your registered listeners' `DataChanged()` from inside `Set` (under a lock). See the source files in `data/binding/` for the standard pattern (`bool.go`, `int.go`, `string.go` are concise reference implementations).
