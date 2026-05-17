# Built-in services

Wails v3 ships several pre-built services under [`v3/pkg/services/`](../../repos-folder/wails/v3/pkg/services/) that you can register the same way as your own.

| Service | Package | Role |
| --- | --- | --- |
| **SQLite** | `services/sqlite` | Drop-in SQLite database access, callable from JS. Uses `modernc.org/sqlite` (pure Go, no CGO). Config takes a DB URI; `:memory:` for in-memory. |
| **KV store** | `services/kvstore` | Persistent key/value store, callable from JS. |
| **File server** | `services/fileserver` | Serve a directory tree as files (useful for very large assets you can't embed). |
| **Log** | `services/log` | Structured logging exposed to JS so the frontend can log into the same Go logger. |
| **Notifications** | `services/notifications` | Native OS toast / banner notifications. Per-OS implementations: `notifications_darwin.go`, `notifications_windows.go`, `notifications_linux.go`, `notifications_ios.go`. |
| **Dock** | `services/dock` | macOS dock badge / progress / context menu. |

## Registration

Identical to your own services:

```go
import "github.com/wailsapp/wails/v3/pkg/services/sqlite"

app := application.New(application.Options{
    Services: []application.Service{
        application.NewService(&MyService{}),
        application.NewServiceWithOptions(
            sqlite.New(),
            application.ServiceOptions{
                Config: sqlite.Config{ DBSource: "app.db" },
            },
        ),
    },
})
```

## Auto-generated bindings include them

The codegen treats built-in services the same as your own — JS imports them from the auto-generated bindings tree, complete with method-ID hashes, doc comments, and types.

## Wails magic comments

Some services use special `//wails:` directives to customize codegen. From [`v3/pkg/services/sqlite/sqlite.go`](../../repos-folder/wails/v3/pkg/services/sqlite/sqlite.go):

```go
//wails:include stmt.js
package sqlite

//wails:inject export {
//wails:inject     ExecContext as Execute,
//wails:inject     QueryContext as Query
//wails:inject };
```

These rename methods on the JS side (`ExecContext` → `Execute`), inject extra JS into the generated module, and include sidecar JS files. Useful when the Go ergonomics differ from what JS callers expect.
