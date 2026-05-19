#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/filedlg.h>
#include <wx/mstream.h>
#include <wx/timer.h>
#include <wx/bmpbndl.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#ifdef __WXMSW__
#include <dwmapi.h>
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#endif

#include <atomic>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>

#include "FlatButton.h"
#include "Metrics.h"
#include "VBar.h"
#include "Worker.h"
#include "ZoomPanel.h"

using nlohmann::json;

namespace {

const wxColour kBg(45, 45, 48);
const wxColour kFg(220, 220, 220);

wxBitmapBundle loadIcon(const wxString& name) {
    wxFileName fn(wxStandardPaths::Get().GetExecutablePath());
    fn.SetFullName("");
    fn.AppendDir("icons");
    fn.SetFullName(name);
    return wxBitmapBundle::FromSVGFile(fn.GetFullPath(), wxSize(20, 20));
}

const std::vector<std::pair<wxString, std::string>> kEngines = {
    {"Auto",             "auto"},
    {"Marker",           "marker"},
    {"Marker + LLM",     "marker_llm"},
    {"VLM (Qwen2.5-VL)", "vlm"},
    {"MinerU",           "mineru"},
};

std::string base64Decode(const std::string& in) {
    static const int8_t table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    };
    std::string out;
    out.reserve(in.size() * 3 / 4);
    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (c == '=' || c == '\n' || c == '\r' || c == ' ') {
            if (c == '=') break;
            continue;
        }
        int8_t d = table[c];
        if (d < 0) continue;
        val = (val << 6) | d;
        valb += 6;
        if (valb >= 0) {
            out.push_back((char)((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

wxImage imageFromPngBytes(const std::string& bytes) {
    wxMemoryInputStream s(bytes.data(), bytes.size());
    wxImage img;
    img.LoadFile(s, wxBITMAP_TYPE_PNG);
    return img;
}

} // namespace

class MainFrame : public wxFrame {
public:
    MainFrame();
    ~MainFrame();

private:
    void OnOpen(wxCommandEvent&);
    void OnPrev(wxCommandEvent&);
    void OnNext(wxCommandEvent&);
    void OnExtractPage(wxCommandEvent&);
    void OnExtractPDF(wxCommandEvent&);
    void OnStop(wxCommandEvent&);
    void OnMetricsTick(wxTimerEvent&);
    void OnKeyDown(wxKeyEvent&);
    void OnKeyUp(wxKeyEvent&);
    void OnClose(wxCloseEvent&);

    void RenderPage(int page);
    void SetBusy(bool busy);
    void UpdateLabel();
    std::string CurrentEngine();

    std::function<void(const json&)> MakeOCRProgress(int page);

    Worker worker_;
    MetricsCollector collector_;

    FlatButton* openBtn_ = nullptr;
    FlatButton* prevBtn_ = nullptr;
    FlatButton* nextBtn_ = nullptr;
    FlatButton* extractPageBtn_ = nullptr;
    FlatButton* extractPDFBtn_ = nullptr;
    FlatButton* stopBtn_ = nullptr;
    wxChoice* engineChoice_ = nullptr;
    wxStaticText* pageLabel_ = nullptr;
    wxStaticText* statusLabel_ = nullptr;
    wxStaticText* fileLabel_ = nullptr;
    wxTextCtrl* textArea_ = nullptr;
    ZoomPanel* preview_ = nullptr;

    VBar* cpuBar_ = nullptr;
    VBar* ramBar_ = nullptr;
    VBar* gpuBar_ = nullptr;
    VBar* vramBar_ = nullptr;
    VBar* tempBar_ = nullptr;

    wxTimer metricsTimer_;

    wxString curPath_;
    int curPage_ = 0;
    int curTotal_ = 0;
    bool ctrlDown_ = false;
    bool spaceDown_ = false;
    std::atomic<bool> busy_{false};
    std::atomic<bool> stopRequested_{false};
};

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "OCR Works", wxDefaultPosition, wxDefaultSize),
      metricsTimer_(this) {

    SetClientSize(FromDIP(wxSize(1200, 800)));

    wxInitAllImageHandlers();

#ifdef __WXMSW__
    SetIcon(wxICON(orcgui));
#endif

    {
        wxFileName iconFn(wxStandardPaths::Get().GetExecutablePath());
        iconFn.SetFullName("");
        iconFn.AppendDir("icons");
        iconFn.SetFullName("OCR_toolbar_icon.svg");
        wxString iconPath = iconFn.GetFullPath();
        if (wxFileExists(iconPath)) {
            wxIconBundle bundle;
            for (int sz : {16, 24, 32, 48, 64, 128, 256}) {
                wxBitmapBundle bb = wxBitmapBundle::FromSVGFile(iconPath, wxSize(sz, sz));
                wxBitmap bmp = bb.GetBitmap(wxSize(sz, sz));
                if (bmp.IsOk()) {
                    wxIcon icon;
                    icon.CopyFromBitmap(bmp);
                    bundle.AddIcon(icon);
                }
            }
            if (!bundle.IsEmpty()) SetIcons(bundle);
        }
    }

    SetBackgroundColour(kBg);
    SetForegroundColour(kFg);

    auto* root = new wxPanel(this);
    root->SetBackgroundColour(kBg);
    root->SetForegroundColour(kFg);

    // Top bar
    auto* topPanel = new wxPanel(root);
    topPanel->SetBackgroundColour(kBg);
    topPanel->SetForegroundColour(kFg);
    openBtn_ = new FlatButton(topPanel, "Open PDF", loadIcon("folder-open.svg"));

    wxArrayString engineNames;
    for (auto& e : kEngines) engineNames.Add(e.first);
    engineChoice_ = new wxChoice(topPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, engineNames);
    engineChoice_->SetSelection(0);

    extractPageBtn_ = new FlatButton(topPanel, "Extract Page", loadIcon("file-text.svg"));
    extractPDFBtn_ = new FlatButton(topPanel, "Extract PDF", loadIcon("files.svg"));
    stopBtn_ = new FlatButton(topPanel, "Stop");
    stopBtn_->SetColors(wxColour(244, 67, 54), wxColour(239, 83, 80), wxColour(211, 47, 47));

    prevBtn_ = new FlatButton(topPanel, "Prev", loadIcon("chevron-left.svg"));
    pageLabel_ = new wxStaticText(topPanel, wxID_ANY, "");
    nextBtn_ = new FlatButton(topPanel, "Next", loadIcon("chevron-right.svg"));

    auto* topSizer = new wxBoxSizer(wxHORIZONTAL);
    topSizer->Add(openBtn_, 0, wxALL, 4);
    topSizer->Add(engineChoice_, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    topSizer->Add(extractPageBtn_, 0, wxALL, 4);
    topSizer->Add(extractPDFBtn_, 0, wxALL, 4);
    topSizer->Add(stopBtn_, 0, wxALL, 4);
    topSizer->AddStretchSpacer();
    topSizer->Add(prevBtn_, 0, wxALL, 4);
    topSizer->Add(pageLabel_, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);
    topSizer->Add(nextBtn_, 0, wxALL, 4);
    topPanel->SetSizer(topSizer);

    // Outer splitter: metrics column | (preview/text inner splitter)
    auto* outerSplitter = new wxSplitterWindow(root, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                               wxSP_3D | wxSP_LIVE_UPDATE);

    // Metrics column (5 thin vertical bars side by side)
    auto* metricsPanel = new wxPanel(outerSplitter, wxID_ANY);
    metricsPanel->SetBackgroundColour(kBg);
    metricsPanel->SetForegroundColour(kFg);
    cpuBar_  = new VBar(metricsPanel, "CPU",  wxColour(80, 170, 220));
    ramBar_  = new VBar(metricsPanel, "RAM",  wxColour(110, 200, 130));
    gpuBar_  = new VBar(metricsPanel, "GPU",  wxColour(220, 140, 80));
    vramBar_ = new VBar(metricsPanel, "VRAM", wxColour(200, 110, 200));
    tempBar_ = new VBar(metricsPanel, "TEMP", wxColour(220, 90, 90));
    auto* metricsSizer = new wxBoxSizer(wxHORIZONTAL);
    metricsSizer->Add(cpuBar_,  1, wxEXPAND | wxALL, 2);
    metricsSizer->Add(ramBar_,  1, wxEXPAND | wxALL, 2);
    metricsSizer->Add(gpuBar_,  1, wxEXPAND | wxALL, 2);
    metricsSizer->Add(vramBar_, 1, wxEXPAND | wxALL, 2);
    metricsSizer->Add(tempBar_, 1, wxEXPAND | wxALL, 2);
    metricsPanel->SetSizer(metricsSizer);

    // Inner splitter: preview | text
    auto* splitter = new wxSplitterWindow(outerSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                          wxSP_3D | wxSP_LIVE_UPDATE);
    preview_ = new ZoomPanel(splitter);
    textArea_ = new wxTextCtrl(splitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_DONTWRAP);
    textArea_->SetBackgroundColour(wxColour(30, 30, 30));
    textArea_->SetForegroundColour(kFg);
    splitter->SplitVertically(preview_, textArea_, FromDIP(700));
    splitter->SetSashGravity(0.7);
    splitter->SetMinimumPaneSize(FromDIP(100));

    // Wire up outer split
    outerSplitter->SplitVertically(metricsPanel, splitter, FromDIP(180));
    outerSplitter->SetSashGravity(0.0);
    outerSplitter->SetMinimumPaneSize(FromDIP(60));

    auto* middleSizer = new wxBoxSizer(wxHORIZONTAL);
    middleSizer->Add(outerSplitter, 1, wxEXPAND);

    // File name banner above the toolbar
    fileLabel_ = new wxStaticText(root, wxID_ANY, "", wxDefaultPosition,
                                  wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
    fileLabel_->SetForegroundColour(kFg);
    {
        wxFont f = fileLabel_->GetFont();
        f.MakeBold();
        f.SetPointSize(f.GetPointSize() + 2);
        fileLabel_->SetFont(f);
    }

    // Status bar text
    statusLabel_ = new wxStaticText(root, wxID_ANY, "");
    statusLabel_->SetForegroundColour(kFg);
    pageLabel_->SetForegroundColour(kFg);

    auto* rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(fileLabel_, 0, wxEXPAND | wxTOP | wxBOTTOM, 4);
    rootSizer->Add(topPanel, 0, wxEXPAND);
    rootSizer->Add(middleSizer, 1, wxEXPAND);
    rootSizer->Add(statusLabel_, 0, wxEXPAND | wxALL, 4);
    root->SetSizer(rootSizer);

    auto* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(root, 1, wxEXPAND);
    SetSizer(frameSizer);

    prevBtn_->Disable();
    nextBtn_->Disable();
    extractPageBtn_->Disable();
    extractPDFBtn_->Disable();
    stopBtn_->Disable();

    openBtn_->Bind(wxEVT_BUTTON, &MainFrame::OnOpen, this);
    prevBtn_->Bind(wxEVT_BUTTON, &MainFrame::OnPrev, this);
    nextBtn_->Bind(wxEVT_BUTTON, &MainFrame::OnNext, this);
    extractPageBtn_->Bind(wxEVT_BUTTON, &MainFrame::OnExtractPage, this);
    extractPDFBtn_->Bind(wxEVT_BUTTON, &MainFrame::OnExtractPDF, this);
    stopBtn_->Bind(wxEVT_BUTTON, &MainFrame::OnStop, this);

    Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& e) {
        int k = e.GetKeyCode();
        if (k == WXK_CONTROL) { ctrlDown_ = true; preview_->SetCtrlDown(true); }
        if (k == WXK_SPACE)   { spaceDown_ = true; preview_->SetSpaceDown(true); }
        if ((k == WXK_LEFT || k == WXK_RIGHT) && !busy_ && curTotal_ > 0
            && wxWindow::FindFocus() != textArea_) {
            if (k == WXK_LEFT && curPage_ > 0) {
                RenderPage(curPage_ - 1);
            } else if (k == WXK_RIGHT && curPage_ < curTotal_ - 1) {
                RenderPage(curPage_ + 1);
            }
            return;
        }
        e.Skip();
    });
    Bind(wxEVT_KEY_UP, [this](wxKeyEvent& e) {
        int k = e.GetKeyCode();
        if (k == WXK_CONTROL) { ctrlDown_ = false; preview_->SetCtrlDown(false); }
        if (k == WXK_SPACE)   { spaceDown_ = false; preview_->SetSpaceDown(false); }
        e.Skip();
    });
    Bind(wxEVT_TIMER, &MainFrame::OnMetricsTick, this);
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);

#ifdef __WXMSW__
    {
        HWND hwnd = (HWND)GetHWND();
        COLORREF black = RGB(0, 0, 0);
        DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &black, sizeof(black));
        DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &black, sizeof(black));
    }
#endif

    collector_.collect();
    metricsTimer_.Start(1000);

    Centre();
}

MainFrame::~MainFrame() = default;

void MainFrame::OnClose(wxCloseEvent& evt) {
    metricsTimer_.Stop();
    worker_.shutdown();
    evt.Skip();
}

std::string MainFrame::CurrentEngine() {
    int sel = engineChoice_->GetSelection();
    if (sel < 0 || sel >= (int)kEngines.size()) return "auto";
    return kEngines[sel].second;
}

void MainFrame::UpdateLabel() {
    if (curTotal_ == 0) {
        pageLabel_->SetLabel("");
    } else {
        pageLabel_->SetLabel(wxString::Format("Page %d of %d", curPage_ + 1, curTotal_));
    }
    pageLabel_->GetParent()->Layout();
}

void MainFrame::SetBusy(bool busy) {
    busy_ = busy;
    if (busy) {
        openBtn_->Disable();
        prevBtn_->Disable();
        nextBtn_->Disable();
        extractPageBtn_->Disable();
        extractPDFBtn_->Disable();
        engineChoice_->Disable();
        stopBtn_->Enable();
        return;
    }
    stopBtn_->Disable();
    openBtn_->Enable();
    engineChoice_->Enable();
    if (curTotal_ == 0) {
        prevBtn_->Disable();
        nextBtn_->Disable();
        extractPageBtn_->Disable();
        extractPDFBtn_->Disable();
        return;
    }
    extractPageBtn_->Enable();
    extractPDFBtn_->Enable();
    prevBtn_->Enable(curPage_ > 0);
    nextBtn_->Enable(curPage_ < curTotal_ - 1);
}

void MainFrame::OnMetricsTick(wxTimerEvent&) {
    MetricsSample s = collector_.collect();
    cpuBar_->Set(s.cpuPct / 100.0, wxString::Format("%.0f%%", s.cpuPct));
    ramBar_->Set(s.ramPct / 100.0, wxString::Format("%.1fG", s.ramUsedGB));
    if (s.hasGPU) {
        gpuBar_->Set(s.gpuPct / 100.0, wxString::Format("%.0f%%", s.gpuPct));
        vramBar_->Set(s.vramPct / 100.0, wxString::Format("%.1fG", s.vramUsedMB / 1024.0));
        tempBar_->Set(s.tempC / 100.0, wxString::Format(L"%.0f°", s.tempC));
    } else {
        gpuBar_->Set(0, "n/a");
        vramBar_->Set(0, "n/a");
        tempBar_->Set(0, "n/a");
    }
}

void MainFrame::OnOpen(wxCommandEvent&) {
    wxFileDialog dlg(this, "Open PDF", "", "",
                     "PDF files (*.pdf)|*.pdf",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) return;
    curPath_ = dlg.GetPath();
    curPage_ = 0;
    curTotal_ = 0;
    textArea_->SetValue("");
    wxString fileName = wxFileName(curPath_).GetFullName();
    SetTitle("OCR Works — " + fileName);
    fileLabel_->SetLabel(fileName);
    fileLabel_->GetParent()->Layout();
    RenderPage(0);
}

void MainFrame::OnPrev(wxCommandEvent&) {
    if (curPage_ > 0) RenderPage(curPage_ - 1);
}

void MainFrame::OnNext(wxCommandEvent&) {
    if (curPage_ < curTotal_ - 1) RenderPage(curPage_ + 1);
}

void MainFrame::RenderPage(int page) {
    if (curPath_.IsEmpty()) return;
    SetBusy(true);

    std::string path = std::string(curPath_.utf8_str());
    std::thread([this, path, page]() {
        json req = {{"cmd", "render"}, {"path", path}, {"page", page}, {"dpi", 120}};
        try {
            json resp = worker_.request(req);
            if (resp.value("type", "") == "error") {
                std::string msg = resp.value("message", "render error");
                CallAfter([this, msg]() {
                    wxMessageBox(msg, "Error", wxOK | wxICON_ERROR, this);
                    SetBusy(false);
                });
                return;
            }
            std::string b64 = resp.value("png_base64", "");
            std::string bytes = base64Decode(b64);
            wxImage img = imageFromPngBytes(bytes);
            int pages = resp.value("pages", 0);

            CallAfter([this, img, pages, page]() {
                if (img.IsOk()) preview_->SetImage(img);
                curPage_ = page;
                if (pages > 0) curTotal_ = pages;
                UpdateLabel();
                textArea_->SetValue("");
                SetBusy(false);
            });
        } catch (const std::exception& e) {
            std::string msg = e.what();
            CallAfter([this, msg]() {
                wxMessageBox(msg, "Error", wxOK | wxICON_ERROR, this);
                SetBusy(false);
            });
        }
    }).detach();
}

std::function<void(const json&)> MainFrame::MakeOCRProgress(int page) {
    auto lastStage = std::make_shared<std::string>();
    return [this, page, lastStage](const json& ev) {
        std::string kind = ev.value("kind", "");

        if (kind == "image") {
            std::string b64 = ev.value("png_base64", "");
            std::string bytes = base64Decode(b64);
            wxImage img = imageFromPngBytes(bytes);
            if (img.IsOk()) {
                CallAfter([this, img]() { preview_->SetImage(img); });
            }
            return;
        }

        std::string text;
        std::string pagePrefix = "page " + std::to_string(page + 1);
        if (kind == "stage") {
            std::string name = ev.value("name", "");
            *lastStage = name;
            text = "● " + pagePrefix + " — " + name;
        } else if (kind == "tqdm") {
            std::string desc = ev.value("desc", "");
            double n = ev.value("n", 0.0);
            double total = ev.value("total", 0.0);
            std::string prefix = lastStage->empty() ? "..." : *lastStage;
            if (total > 0) {
                text = "● " + pagePrefix + " — " + prefix + " — " + desc
                       + " " + std::to_string((int)n) + "/" + std::to_string((int)total);
            } else if (!desc.empty()) {
                text = "● " + pagePrefix + " — " + prefix + " — " + desc;
            } else {
                text = "● " + pagePrefix + " — " + prefix;
            }
        } else {
            return;
        }
        CallAfter([this, text]() { statusLabel_->SetLabel(text); });
    };
}

void MainFrame::OnStop(wxCommandEvent&) {
    if (!busy_) return;
    stopRequested_ = true;
    statusLabel_->SetLabel("stopping...");
    worker_.cancel();
}

void MainFrame::OnExtractPage(wxCommandEvent&) {
    if (curPath_.IsEmpty()) return;
    std::string engine = CurrentEngine();
    int page = curPage_;
    std::string path = std::string(curPath_.utf8_str());

    stopRequested_ = false;
    SetBusy(true);
    textArea_->SetValue("");
    statusLabel_->SetLabel(wxString::Format("starting (%s)...", engine));

    auto progress = MakeOCRProgress(page);
    std::thread([this, path, page, engine, progress]() {
        json req = {{"cmd", "ocr"}, {"path", path}, {"page", page}, {"engine", engine}};
        try {
            json resp = worker_.request(req, progress);
            if (resp.value("type", "") == "error") {
                std::string msg = resp.value("message", "ocr error");
                CallAfter([this, msg]() {
                    wxMessageBox(msg, "Error", wxOK | wxICON_ERROR, this);
                    textArea_->SetValue("");
                    statusLabel_->SetLabel("");
                    SetBusy(false);
                });
                return;
            }
            std::string text = resp.value("text", "");
            std::string savedTo = resp.value("saved_to", "");
            std::string used = resp.value("engine", "");
            CallAfter([this, text, savedTo, used]() {
                textArea_->SetValue(wxString::FromUTF8(text));
                std::string prefix = used.empty() ? "" : "[" + used + "] ";
                if (!savedTo.empty()) {
                    statusLabel_->SetLabel(wxString::FromUTF8(prefix + "saved → " + savedTo));
                } else {
                    statusLabel_->SetLabel(wxString::FromUTF8(prefix + "done"));
                }
                SetBusy(false);
            });
        } catch (const std::exception& e) {
            std::string msg = e.what();
            bool cancelled = stopRequested_.load();
            CallAfter([this, msg, cancelled]() {
                if (!cancelled) {
                    wxMessageBox(msg, "Error", wxOK | wxICON_ERROR, this);
                    textArea_->SetValue("");
                    statusLabel_->SetLabel("");
                } else {
                    statusLabel_->SetLabel("stopped");
                }
                SetBusy(false);
            });
        }
    }).detach();
}

void MainFrame::OnExtractPDF(wxCommandEvent&) {
    if (curPath_.IsEmpty() || curTotal_ == 0) return;
    std::string engine = CurrentEngine();
    int total = curTotal_;
    std::string path = std::string(curPath_.utf8_str());

    stopRequested_ = false;
    SetBusy(true);
    textArea_->SetValue("");
    statusLabel_->SetLabel(wxString::Format("starting PDF extract (%s): %d pages", engine, total));

    std::thread([this, path, total, engine]() {
        std::ostringstream allText;
        std::string lastSavedTo;
        bool failed = false;

        for (int page = 0; page < total && !failed; ++page) {
            if (stopRequested_.load()) { failed = true; break; }
            CallAfter([this, page, total, engine]() {
                curPage_ = page;
                UpdateLabel();
                statusLabel_->SetLabel(wxString::Format(
                    "extract PDF (%s): page %d of %d", engine, page + 1, total));
            });

            auto progress = MakeOCRProgress(page);
            try {
                json req = {{"cmd", "ocr"}, {"path", path}, {"page", page}, {"engine", engine}};
                json resp = worker_.request(req, progress);
                if (resp.value("type", "") == "error") {
                    std::string msg = resp.value("message", "ocr error");
                    bool cancelled = stopRequested_.load();
                    CallAfter([this, msg, cancelled]() {
                        if (!cancelled) wxMessageBox(msg, "Error", wxOK | wxICON_ERROR, this);
                        statusLabel_->SetLabel(cancelled ? "stopped" : "");
                        SetBusy(false);
                    });
                    failed = true;
                    break;
                }
                std::string text = resp.value("text", "");
                std::string savedTo = resp.value("saved_to", "");
                std::string used = resp.value("engine", "");
                if (!savedTo.empty()) lastSavedTo = savedTo;
                if (allText.tellp() > 0) allText << "\n\n";
                allText << "# Page " << (page + 1) << "\n\n" << text;

                std::string snapshot = allText.str();
                std::string suffix = used.empty() ? "" : " [" + used + "]";
                int pageOneIndexed = page + 1;
                CallAfter([this, snapshot, pageOneIndexed, total, suffix]() {
                    textArea_->SetValue(wxString::FromUTF8(snapshot));
                    statusLabel_->SetLabel(wxString::Format(
                        "extract PDF: completed page %d of %d%s",
                        pageOneIndexed, total, suffix));
                });
            } catch (const std::exception& e) {
                std::string msg = e.what();
                bool cancelled = stopRequested_.load();
                CallAfter([this, msg, cancelled]() {
                    if (!cancelled) wxMessageBox(msg, "Error", wxOK | wxICON_ERROR, this);
                    statusLabel_->SetLabel(cancelled ? "stopped" : "");
                    SetBusy(false);
                });
                failed = true;
                break;
            }
        }

        if (failed && stopRequested_.load()) {
            CallAfter([this]() {
                statusLabel_->SetLabel("stopped");
                SetBusy(false);
            });
        } else if (!failed) {
            std::string savedTo = lastSavedTo;
            CallAfter([this, savedTo]() {
                if (!savedTo.empty()) {
                    statusLabel_->SetLabel(wxString::FromUTF8("saved → " + savedTo));
                } else {
                    statusLabel_->SetLabel("done");
                }
                SetBusy(false);
            });
        }
    }).detach();
}

class App : public wxApp {
public:
    bool OnInit() override {
#ifdef __WXMSW__
        MSWEnableDarkMode(DarkMode_Always);
#endif
        auto* frame = new MainFrame();
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(App);
