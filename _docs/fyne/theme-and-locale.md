# Theme, locale, accessibility

## Theme ([theme/](../../repos-folder/fyne/theme/))

A `fyne.Theme` maps colors, sizes, fonts, and icons to symbolic names. Widgets read from `theme.<XYZ>()` helpers at render time, so a theme change re-skins the whole app live.

### Built-in themes

```go
theme.DefaultTheme()       // adaptive to user preference
theme.DarkTheme()          // deprecated; ignores user preference
theme.LightTheme()         // deprecated; ignores user preference
```

`VariantDark` and `VariantLight` are exported constants representing the two variants Fyne supports. A theme can decide what to do with the variant; the built-in theme switches palettes.

### Selecting a custom theme

```go
a := app.New()
a.Settings().SetTheme(myTheme)
```

`a.Settings()` exposes the global theme and the user's scale preference. Switching at runtime causes every widget that's currently in the canvas tree to call `Refresh()`.

### Implementing a theme

```go
type myTheme struct{}

func (m *myTheme) Color(name fyne.ThemeColorName, variant fyne.ThemeVariant) color.Color { ... }
func (m *myTheme) Font(style fyne.TextStyle) fyne.Resource { ... }
func (m *myTheme) Icon(name fyne.ThemeIconName) fyne.Resource { ... }
func (m *myTheme) Size(name fyne.ThemeSizeName) float32 { ... }
```

Common color names (from [theme/color.go](../../repos-folder/fyne/theme/color.go)): `ColorNameBackground`, `ColorNameForeground`, `ColorNamePrimary`, `ColorNameError`, `ColorNameInputBackground`, `ColorNameButton`, `ColorNameDisabled`, `ColorNameHover`, `ColorNameSelection`, `ColorNamePlaceHolder`, `ColorNameSeparator`, `ColorNameScrollBar`, `ColorNameShadow`, `ColorNameInputBorder`, `ColorNameFocus`, `ColorNameMenuBackground`, `ColorNameHeaderBackground`, `ColorNameOverlayBackground`. Plus stylistic primaries (red/green/blue/yellow/orange/purple/pink/brown/gray) for variant accents (e.g. importance levels in buttons).

Size names: `SizeNameText`, `SizeNamePadding`, `SizeNameInnerPadding`, `SizeNameInputBorder`, `SizeNameInputRadius`, `SizeNameScrollBar`, `SizeNameSeparatorThickness`, etc.

Icon names: a large set (`IconNameHome`, `IconNameSettings`, `IconNameFile*`, `IconNameContent*`, ...) — see [theme/icons.go](../../repos-folder/fyne/theme/icons.go).

### Themes from JSON ([theme/json.go](../../repos-folder/fyne/theme/json.go))

```go
data := `{"Colors": {"primary": "#aabbcc"}, "Sizes": {"text": 14}}`
t, err := theme.FromJSON(data)
```

Convenient for letting users supply a theme file without recompiling.

### Themed test app ([theme/themedtestapp_test.go](../../repos-folder/fyne/theme/themedtestapp_test.go))

For widget authors: a tiny helper that runs a test app under a configurable theme so the widget's `Refresh` path can be exercised.

## Fonts and icons

`bundled-fonts.go` ships the default Inter / Source Code Pro family compiled into the binary. `theme.TextFont()` returns the regular face; `theme.TextBoldFont()`, `theme.TextItalicFont()`, `theme.TextMonospaceFont()`, `theme.SymbolFont()`, `theme.TextBoldItalicFont()` etc. cover the variants.

`bundled-emoji.go` / `unbundled-emoji.go` provide an emoji fallback face. The unbundled variant pulls from the OS where available to keep binary size down.

`bundled-icons.go` holds the Material-design-style icon set. To use a theme icon:

```go
icon := theme.HomeIcon()
img := widget.NewIcon(icon)
```

## Locale and i18n ([lang/](../../repos-folder/fyne/lang/))

Fyne ships translations for ~30 locales in `lang/translations/`. Apps initialize the bundle:

```go
import "fyne.io/fyne/v2/lang"

func main() {
    lang.AddTranslationsFS(myEmbedFS, "translations")
    lang.X("greeting.hello", "Hello")          // localized lookup; second arg is the fallback
}
```

`lang.X(id, fallback)` and `lang.N(id, plural, count, fallback)` are the runtime lookups. Translations are stored as TOML files (one per locale).

`a.Settings().Locale()` reports the current locale (sniffed from OS). `lang.SystemLocale()` ([locale.go](../../repos-folder/fyne/locale.go)) is the underlying call.

The widget package uses these for its own strings ("OK", "Cancel", date formats, etc.) so file pickers, dialogs, and forms are localized automatically.

## Accessibility ([accessibility.go](../../repos-folder/fyne/accessibility.go))

Since 2.8, every `CanvasObject` may implement:

```go
type Accessible interface {
    AccessibilityLabel() string
    AccessibilityRole() AccessibleRole
}
```

Roles (a small enum: `AccessibleRoleNone`, `AccessibleRoleButton`, `AccessibleRoleText`, `AccessibleRoleHeading`, `AccessibleRoleContainer`, `AccessibleRoleImage`, `AccessibleRoleList`, `AccessibleRoleListItem`, ...) feed platform screen readers (VoiceOver, TalkBack, NVDA, JAWS) through the driver.

`*fyne.Container` returns `"Container"` / `AccessibleRoleContainer` by default. Stock widgets implement appropriate labels and roles; custom widgets should too if they're meant to be reachable.
