# Hello World

The shortest functional wxWidgets program, from
`docs/doxygen/overviews/helloworld.h` upstream:

```cpp
#include <wx/wx.h>

class MyApp : public wxApp
{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxFrame
{
public:
    MyFrame();

private:
    void OnHello(wxCommandEvent&);
    void OnExit(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);
};

enum { ID_Hello = 1 };

bool MyApp::OnInit()
{
    auto* frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Hello World")
{
    auto* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H", "Help string");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    auto* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    auto* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit,  this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent&)  { Close(true); }
void MyFrame::OnAbout(wxCommandEvent&) { wxMessageBox("Hello World example",
                                                      "About",
                                                      wxOK | wxICON_INFORMATION); }
void MyFrame::OnHello(wxCommandEvent&) { wxLogMessage("Hello from wxWidgets!"); }
```

## Things to notice

- `wxIMPLEMENT_APP(MyApp)` generates the platform-correct entry point
  (`WinMain` on MSW, `main` elsewhere) and instantiates `MyApp`.
- `wxApp::OnInit()` returning `false` aborts startup; returning `true`
  hands control to `wxApp::OnRun()` which runs the event loop.
- Frames are created **hidden** — call `Show()` explicitly.
- No `delete`s: wxWidgets owns the window tree and destroys children with
  their parent. Top-level frames are deleted when closed.
- `wxID_EXIT` / `wxID_ABOUT` are platform-special (e.g. moved to the Apple
  menu on macOS) — prefer them over custom IDs.

The variant living in `samples/minimal/minimal.cpp` adds an event table
(`wxBEGIN_EVENT_TABLE` / `EVT_MENU` / `wxEND_EVENT_TABLE`) instead of
`Bind()`. Both styles are supported — see [event-system.md](event-system.md).
