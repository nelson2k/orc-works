# Events

wxWidgets has two coexisting styles for hooking up event handlers.

## 1. `Bind()` (preferred, modern)

```cpp
Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
Bind(wxEVT_BUTTON, [=](wxCommandEvent&) { /* ... */ }, myBtn->GetId());
```

Pros: lambdas, non-member functions, member functions, anything callable;
established at runtime; can be `Unbind()`-ed.

Signature: `Bind(eventType, handler, [userData|this], [id], [lastId])`.

## 2. Static event tables (classic)

```cpp
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
wxEND_EVENT_TABLE()

class MyFrame : public wxFrame {
    void OnExit(wxCommandEvent&);
    wxDECLARE_EVENT_TABLE();
};
```

Pros: very compact for many menu items; defined declaratively at the top of
a file.

## Event types worth knowing

- `wxEVT_MENU` — menu item selected (also fires for accelerators)
- `wxEVT_BUTTON` — `wxButton` click
- `wxEVT_TEXT`, `wxEVT_TEXT_ENTER` — text control changes / Enter pressed
- `wxEVT_CHECKBOX`, `wxEVT_RADIOBUTTON`, `wxEVT_CHOICE`, `wxEVT_COMBOBOX`
- `wxEVT_CLOSE_WINDOW` — frame closing; can be vetoed
- `wxEVT_SIZE`, `wxEVT_PAINT`, `wxEVT_ERASE_BACKGROUND`
- `wxEVT_LEFT_DOWN` / `_UP` / `_DCLICK`, `wxEVT_MOTION`, `wxEVT_MOUSEWHEEL`
- `wxEVT_KEY_DOWN`, `wxEVT_KEY_UP`, `wxEVT_CHAR`
- `wxEVT_TIMER` — `wxTimer` fires
- `wxEVT_IDLE` — fired when the event queue is empty

## Cross-thread

From a worker thread, never touch GUI objects directly. Use:

- `CallAfter([=] { /* run on GUI thread */ });`
- `wxQueueEvent(handler, new MyEvent(...));`

## Custom events

Declare with `wxDECLARE_EVENT(MY_EVENT, wxCommandEvent);` and define with
`wxDEFINE_EVENT(MY_EVENT, wxCommandEvent);`. Bind/queue them like the
built-in ones.
