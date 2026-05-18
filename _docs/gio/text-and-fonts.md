# Text and fonts

Packages: [gioui.org/text](../../repos-folder/gio/text/), [gioui.org/font](../../repos-folder/gio/font/), [font/gofont](../../repos-folder/gio/font/gofont/), [font/opentype](../../repos-folder/gio/font/opentype/)

Gio uses [go-text/typesetting](https://github.com/go-text/typesetting) under the hood for shaping. The `text.Shaper` is the gate between text strings and glyph runs.

## `text.Shaper`

```go
import "gioui.org/text"

shaper := text.NewShaper(
    text.WithCollection(gofont.Collection()),   // pin a font collection
    text.NoSystemFonts(),                        // skip the OS fallback chain
)
```

Constructors:

- `text.NewShaper(opts ...ShaperOption) *Shaper`
- Options:
  - `text.NoSystemFonts()` — don't consult OS-installed fonts
  - `text.WithCollection(collection []FontFace)` — explicit list of typefaces

Shapers cache parsed font data and shaped glyph runs. One per window is the recommended sharing granularity.

## Parameters and glyphs

```go
type Parameters struct {
    Font      font.Font     // family, style, weight
    PxPerEm   fixed.Int26_6 // size in pixels
    Alignment Alignment     // Start, End, Middle, ...
    Locale    system.Locale
    MaxLines  int
    LineHeight unit.Sp
    MaxWidth  int
    MinWidth  int
    PixelsPerEm fixed.Int26_6
    Truncator string
    WrapPolicy WrapPolicy
    ...
}

type Glyph struct {
    X, Y         fixed.Int26_6
    Advance      fixed.Int26_6
    Ascent, Descent fixed.Int26_6
    Bounds       fixed.Rectangle26_6
    ID           GlyphID
    Flags        Flags
    ...
}
```

`WrapPolicy`: `WrapHeuristically`, `WrapGraphemes`, `WrapWords` (constants in [text.go](../../repos-folder/gio/text/text.go)).

`Flags`: bit set on glyphs marking line/cluster boundaries, RTL runs, etc.

## Using the shaper from a widget

You normally won't call the shaper directly — `widget.Editor`, `widget.Selectable`, and `material.LabelStyle` do it for you. The interface:

```go
shaper.LayoutString(params, "Hello world")
for g, ok := shaper.NextGlyph(); ok; g, ok = shaper.NextGlyph() { ... }
```

A shaped run can be drawn by stepping the glyph iterator and calling the [internal] text-rendering ops emitted by the shaper.

## Fonts

```go
import "gioui.org/font"

type Typeface string

type Font struct {
    Typeface Typeface  // "Go", "Inter", ...
    Style    Style     // Regular, Italic
    Weight   Weight    // Normal, Medium, Bold, Black, Thin, ...
}
```

Styles: `Regular`, `Italic`. Weights: `Thin`, `ExtraLight`, `Light`, `Normal`, `Medium`, `SemiBold`, `Bold`, `ExtraBold`, `Black`.

### `font/gofont`

Embeds the Go font family (Go, Go Mono) as a `text.Collection`:

```go
import "gioui.org/font/gofont"

shaper := text.NewShaper(text.WithCollection(gofont.Collection()))
```

Convenience faces:

- `gofont.Regular`, `gofont.Bold`, `gofont.Italic`, `gofont.BoldItalic`
- `gofont.Medium`, `gofont.MediumItalic`
- `gofont.Mono*` variants

### `font/opentype`

Loads OpenType fonts from raw bytes:

```go
import "gioui.org/font/opentype"

face, err := opentype.Parse(ttfBytes)
collection := []text.FontFace{
    {Font: font.Font{Typeface: "Inter", Weight: font.Normal}, Face: face},
    {Font: font.Font{Typeface: "Inter", Weight: font.Bold},   Face: faceBold},
}
shaper := text.NewShaper(text.WithCollection(collection))
```

`opentype.ParseCollection(bytes)` handles TrueType / OpenType collections.

## Alignment

```go
type Alignment uint8
const (
    Start    Alignment = iota
    End
    Middle
    SpanLeft
    SpanRight
    SpanStart
    SpanEnd
)
```

`Start` / `End` are locale-aware (right-aligned in RTL locales). `SpanLeft` / `SpanRight` are absolute.

## Locale

`system.Locale{Language, Direction}` (in [io/system](../../repos-folder/gio/io/system/)) flows through the layout context (`gtx.Locale`) and is consumed by shaping to drive bidirectional layout, line-break rules, and digit shaping.
