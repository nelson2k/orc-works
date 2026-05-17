# Bindings — Go methods callable from JS

In v3, you register **services** — Go structs whose public methods become JS-callable. Wails generates typed JS/TS files for every service so the frontend gets autocomplete and type-safety.

## Defining a service (Go)

From [`v3/examples/binding/GreetService.go`](../../repos-folder/wails/v3/examples/binding/GreetService.go):

```go
type GreetService struct{}

// Greet greets a person
func (*GreetService) Greet(name string, counts ...int) string { ... }

// GreetPerson greets a person
func (srv *GreetService) GreetPerson(person data.Person) string { ... }

// GetPerson returns a person with the given name.
func (srv *GreetService) GetPerson(name string) data.Person { ... }
```

Register it in `main.go`:

```go
app := application.New(application.Options{
    Services: []application.Service{
        application.NewService(&GreetService{}),
    },
    // ...
})
```

The exported methods become callable from JS. Variadic args (`counts ...int`), custom struct args (`data.Person`), and custom struct returns are all supported — the generator handles them.

## Generated JS (auto)

From the same example's [`assets/bindings/.../greetservice.js`](../../repos-folder/wails/v3/examples/binding/assets/bindings/github.com/wailsapp/wails/v3/examples/binding/greetservice.js):

```js
import {Call as $Call, CancellablePromise as $CancellablePromise, Create as $Create} from "/wails/runtime.js";
import * as data$0 from "./data/models.js";

/**
 * Greet greets a person
 * @param {string} name
 * @param {number[]} counts
 * @returns {$CancellablePromise<string>}
 */
export function Greet(name, ...counts) {
    return $Call.ByID(1411160069, name, counts);
}
```

Notes:

- Calls are addressed by a **stable hash ID** (`1411160069`) generated from the method signature, not by name. This makes minification safe and gives obfuscation by default.
- Returns are `CancellablePromise` — call `.cancel()` on the JS side to abort an in-flight call (Go side gets a context cancellation).
- Doc comments from the Go method become JSDoc on the JS side.
- Custom types are emitted as separate `models.js` / `models.ts` files alongside the service file.

## TypeScript variant

The same generator emits `.ts` files when the project is TS — `data.Person` becomes a TypeScript interface, method signatures are fully typed, and `CancellablePromise<Person>` is correctly parameterized.

## Calling from JS

```js
import {Greet} from "./bindings/.../greetservice.js";

const reply = await Greet("Alice", 3, 5);   // "Hello 3, 5 times Alice"
```

## Error handling

Service methods can return `(T, error)`. The error path becomes a rejected promise on the JS side. Custom JSON serialization for errors is configurable via `Options.MarshalError` — useful when you want to send structured error objects (codes, fields) instead of plain strings.

## BindAliases

`Options.BindAliases map[uint32]uint32` lets you alias method IDs — useful when refactoring a service signature without breaking existing JS that's already loaded.
