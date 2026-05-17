# Mobile (iOS + Android, v3 alpha)

v3 brings iOS and Android backends. v2 has neither. Source-of-truth design docs:

- [`v3/IOS_ARCHITECTURE.md`](../../repos-folder/wails/v3/IOS_ARCHITECTURE.md)
- [`v3/ANDROID_ARCHITECTURE.md`](../../repos-folder/wails/v3/ANDROID_ARCHITECTURE.md)

Both are explicitly designed around **battery efficiency** and **no listening ports** — asset serving stays in-process, the JS↔Go bridge is direct (not HTTP).

## iOS

- **WebView**: WKWebView wrapped in a `WailsViewController`.
- **Bridge**: CGO calling Objective-C. The Go runtime runs in the same process as the iOS app.
- **Hosting**: UIKit `UIApplication` + `WailsViewController` + WKWebView.
- **Max 2 WebViews concurrently** (one primary, one for transitions) — explicit battery decision.
- **Asset serving**: native iOS file APIs, no HTTP. The WebView resolves a custom URL scheme (`wails://`) to in-process bytes.
- Source files: `pkg/application/application_ios.go`, `application_ios.h`, `application_ios.m`, `application_ios_delegate.{h,m}`.
- Build path: `build_ios.sh`, `verify-ios-setup.sh`, `fix-darwin-ios-constraints.sh`.

## Android

- **WebView**: Android's native WebView (Chromium-based via Play Services).
- **Bridge**: JNI (Java Native Interface). Go is compiled as `-buildmode=c-shared` into a `.so` library loaded by the host Activity at runtime.
- **Hosting**: Java/Kotlin Activity hosts the WebView; the Activity exposes Java methods to JS via `addJavascriptInterface`, and forwards them to Go through JNI.
- **Asset serving**: `WebViewAssetLoader` from AndroidX — no HTTP server.
- **Design pattern**: follows Fyne's `gomobile` shared-library approach, not gomobile bindings themselves.
- Source files: `pkg/application/application_android.go`, `application_android_nocgo.go`.

## What this changes about Wails architecturally

The mobile push has reshaped v3 even on desktop:

- Asset serving without a port (was an option in v2; now the default in v3).
- A simpler IPC bridge (no HTTP roundtrip → easier to keep portable across desktop and mobile backends).
- Service registration as a first-class API — instead of `Bind` ad-hoc to a struct, you `application.NewService(&T{})` which the framework can route to the right transport per platform.

## What's there vs what's missing

The architecture docs read like polished design — but v3 is alpha. Expect:

- API churn between releases (Bind* aliases exist for migration).
- Mobile-only services (`notifications_ios.go`) that don't have full feature parity with desktop yet.
- Build tooling gaps — `wails3 build --target ios` works but pre-flight checks are minimal (hence the helper shell scripts).
