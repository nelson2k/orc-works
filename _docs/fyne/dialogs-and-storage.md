# Dialogs and storage

## Stock dialogs ([dialog/](../../repos-folder/fyne/dialog/))

All dialogs take the parent window as their last argument and are shown with `.Show()`. They're modal over the parent canvas, not OS-level modal — Fyne renders them in the same window.

### Information / confirmation / form

```go
dialog.ShowInformation("Title", "Body text", w)
dialog.ShowError(errors.New("nope"), w)

dialog.ShowConfirm("Delete?", "Are you sure?", func(ok bool) {
    if ok { ... }
}, w)

dialog.ShowCustomConfirm("Title", "Save", "Cancel", customContent,
    func(ok bool) { ... }, w)

dialog.NewForm("Add user", "Add", "Cancel",
    []*widget.FormItem{
        {Text: "Name", Widget: nameEntry},
        {Text: "Email", Widget: emailEntry},
    },
    func(ok bool) { ... },
    w,
).Show()
```

### File and folder pickers

```go
dialog.ShowFileOpen(func(reader fyne.URIReadCloser, err error) {
    if reader == nil { return }   // user cancelled
    defer reader.Close()
    // read bytes from reader
}, w)

dialog.ShowFileSave(func(writer fyne.URIWriteCloser, err error) { ... }, w)
dialog.ShowFolderOpen(func(uri fyne.ListableURI, err error)   { ... }, w)
```

Per-platform implementations (`file_xdg.go`, `file_darwin.go`, `file_windows.go`, `file_mobile.go`, `file_wasm.go`, `file_xdg_flatpak.go`) dispatch to the native picker where possible. Flatpak builds go through the XDG document portal so the app sees the file via a sandbox-friendly URI.

### Color picker

```go
dialog.ShowColorPicker("Pick a color", "Choose", func(c color.Color) { ... }, w)
```

The wheel + sliders implementation is configurable; see `color_picker.go`.

### Progress and entry

```go
prog := dialog.NewProgress("Working", "Please wait", w)
prog.Show()
prog.SetValue(0.5)        // 0..1
prog.Hide()

dialog.ShowEntry("Enter name", "Name:", func(name string) { ... }, w)
```

`dialog.NewProgressInfinite` for indeterminate progress.

### Custom dialogs

```go
d := dialog.NewCustom("Title", "Dismiss", customContent, w)
d.Resize(fyne.NewSize(400, 300))
d.Show()
```

`dialog.NewCustomWithoutButtons` for chromeless modals.

## Storage and URI abstraction ([storage/](../../repos-folder/fyne/storage/))

Fyne abstracts the file system behind a URI-style API so the same code works on desktop (real paths), mobile (sandboxed paths), and web (IndexedDB-backed virtual paths).

### `fyne.URI`

```go
uri := storage.NewFileURI("/path/to/file.txt")
uri.Scheme()       // "file"
uri.Authority()    // ""
uri.Path()         // "/path/to/file.txt"
uri.Name()         // "file.txt"
uri.Extension()    // ".txt"
uri.MimeType()     // "text/plain"
```

### Read/write through repositories

```go
reader, err := storage.Reader(uri)        // fyne.URIReadCloser
writer, err := storage.Writer(uri)        // fyne.URIWriteCloser
exists, err := storage.Exists(uri)
err = storage.Delete(uri)
err = storage.Copy(srcURI, dstURI)
err = storage.Move(srcURI, dstURI)
```

### Listable URIs (directories)

```go
listable, err := storage.ListerForURI(folderURI)
children, err := listable.List()
ok := storage.CanList(uri)
```

### Repositories ([storage/repository/](../../repos-folder/fyne/storage/repository))

A `Repository` is plug-in storage backend. Out of the box: file (`file://`), HTTP/HTTPS (`http://`), in-memory (`mem://`). Apps can register their own to handle custom URI schemes (e.g. `gdrive://` for a cloud integration). The default file repository per OS lives in `internal/repository/`.

### App-scoped storage

`a.Storage()` returns a `fyne.Storage` that's rooted in an app-private directory. Use it for per-app data files that should be portable across platforms:

```go
s := a.Storage()
root := s.RootURI()                       // a fyne.ListableURI
child, _ := storage.Child(root, "data.json")
w, _ := storage.Writer(child)
w.Write([]byte(...)); w.Close()
```

On mobile this maps to the per-app sandbox; on desktop it's typically `$XDG_DATA_HOME/<app-id>/` or the OS equivalent.

### Resources

`fyne.Resource` is a `(name, bytes)` pair for bundled assets — images, icons, fonts. Use `fyne package` (or `go:generate` with `fyne bundle`) to embed files as Go source:

```go
//go:generate fyne bundle -o bundled.go icon.png

resourceIcon := bundledIconPng
img := canvas.NewImageFromResource(resourceIcon)
```

## Validation ([data/validation/](../../repos-folder/fyne/data/validation/))

Pre-built validators that conform to `fyne.StringValidator`:

```go
emailValidator := validation.NewRegexp(`^[^@]+@[^@]+\.[^@]+$`, "invalid email")
timeValidator  := validation.NewTime("15:04:05")
allValidator   := validation.NewAllStrings(emailValidator, ...) // composition

entry := widget.NewEntry()
entry.Validator = emailValidator
// invalid input is shown inline; Forms refuse to submit if any field is invalid
```

`widget.Form` and `widget.FormItem` integrate with these — if any field's validator returns an error the form's submit button is disabled.
