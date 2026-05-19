#include "ZoomPanel.h"

#include <wx/dcbuffer.h>
#include <wx/settings.h>

#include <algorithm>

namespace {
constexpr double kZoomStep = 1.1;
constexpr int kZoomMinSide = 100;
constexpr int kZoomMaxSide = 8000;
}

ZoomPanel::ZoomPanel(wxWindow* parent)
    : wxScrolled<wxPanel>(parent, wxID_ANY) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetScrollRate(20, 20);
    SetBackgroundColour(*wxBLACK);

    Bind(wxEVT_PAINT, &ZoomPanel::OnPaint, this);
    Bind(wxEVT_MOUSEWHEEL, &ZoomPanel::OnMouseWheel, this);
    Bind(wxEVT_LEFT_DOWN, &ZoomPanel::OnMouseDown, this);
    Bind(wxEVT_LEFT_UP, &ZoomPanel::OnMouseUp, this);
    Bind(wxEVT_MOTION, &ZoomPanel::OnMouseMotion, this);
    Bind(wxEVT_ENTER_WINDOW, &ZoomPanel::OnEnter, this);
}

void ZoomPanel::SyncSpaceFromKeyboard() {
    bool actual = wxGetKeyState(WXK_SPACE);
    if (actual != spaceDown_) {
        spaceDown_ = actual;
        if (!actual && dragging_) {
            dragging_ = false;
            if (HasCapture()) ReleaseMouse();
        }
        UpdateCursor();
    }
}

void ZoomPanel::UpdateCursor() {
    if (dragging_)        SetCursor(wxCursor(wxCURSOR_SIZING));
    else if (spaceDown_)  SetCursor(wxCursor(wxCURSOR_HAND));
    else                  SetCursor(wxNullCursor);
}

void ZoomPanel::SetImage(const wxImage& img) {
    image_ = img;
    if (image_.IsOk() && scale_ == 1.0) {
        scale_ = 1.0;
    }
    RebuildBitmap();
    UpdateVirtualSize();
    Refresh(false);
}

void ZoomPanel::FitWidth() {
    if (!image_.IsOk()) return;
    wxSize client = GetClientSize();
    // Reserve space for the vertical scrollbar so the image actually fits.
    int sbW = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
    if (sbW <= 0) sbW = FromDIP(16);
    int availW = client.x - sbW;
    if (availW <= 0 || image_.GetWidth() <= 0) return;

    double s = double(availW) / double(image_.GetWidth());
    int nw = int(image_.GetWidth() * s);
    int nh = int(image_.GetHeight() * s);
    if (nw < kZoomMinSide || nh < kZoomMinSide) return;
    if (nw > kZoomMaxSide || nh > kZoomMaxSide) return;

    scale_ = s;
    Scroll(0, 0);
    RebuildBitmap();
    UpdateVirtualSize();
    Refresh(false);
}

void ZoomPanel::SetSpaceDown(bool v) {
    if (spaceDown_ == v) return;
    spaceDown_ = v;
    if (!v && dragging_) {
        dragging_ = false;
        if (HasCapture()) ReleaseMouse();
    }
    UpdateCursor();
}

void ZoomPanel::RebuildBitmap() {
    if (!image_.IsOk()) {
        bitmap_ = wxBitmap();
        return;
    }
    int w = std::max(1, (int)(image_.GetWidth() * scale_));
    int h = std::max(1, (int)(image_.GetHeight() * scale_));
    wxImage scaled = image_.Scale(w, h, wxIMAGE_QUALITY_BILINEAR);
    bitmap_ = wxBitmap(scaled);
}

void ZoomPanel::UpdateVirtualSize() {
    if (bitmap_.IsOk()) {
        SetVirtualSize(bitmap_.GetWidth(), bitmap_.GetHeight());
    } else {
        SetVirtualSize(0, 0);
    }
}

void ZoomPanel::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    DoPrepareDC(dc);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();
    if (bitmap_.IsOk()) {
        dc.DrawBitmap(bitmap_, 0, 0, false);
    }
}

void ZoomPanel::OnMouseWheel(wxMouseEvent& evt) {
    ctrlDown_ = wxGetKeyState(WXK_CONTROL) || evt.ControlDown();
    if (ctrlDown_ && image_.IsOk()) {
        double f = evt.GetWheelRotation() > 0 ? kZoomStep : 1.0 / kZoomStep;
        double ns = scale_ * f;
        int nw = (int)(image_.GetWidth() * ns);
        int nh = (int)(image_.GetHeight() * ns);
        if (nw < kZoomMinSide || nh < kZoomMinSide) return;
        if (nw > kZoomMaxSide || nh > kZoomMaxSide) return;
        scale_ = ns;
        RebuildBitmap();
        UpdateVirtualSize();
        Refresh(false);
        return;
    }
    evt.Skip();
}

void ZoomPanel::OnMouseDown(wxMouseEvent& evt) {
    SyncSpaceFromKeyboard();
    if (spaceDown_) {
        dragging_ = true;
        dragLast_ = evt.GetPosition();
        CaptureMouse();
        UpdateCursor();
    }
    evt.Skip();
}

void ZoomPanel::OnMouseUp(wxMouseEvent& evt) {
    if (dragging_) {
        dragging_ = false;
        if (HasCapture()) ReleaseMouse();
        UpdateCursor();
    }
    evt.Skip();
}

void ZoomPanel::OnMouseMotion(wxMouseEvent& evt) {
    SyncSpaceFromKeyboard();
    if (dragging_ && spaceDown_) {
        wxPoint p = evt.GetPosition();
        wxPoint d = p - dragLast_;
        dragLast_ = p;

        int xu, yu;
        GetScrollPixelsPerUnit(&xu, &yu);
        int sx = GetScrollPos(wxHORIZONTAL);
        int sy = GetScrollPos(wxVERTICAL);
        Scroll(sx - d.x / (xu ? xu : 1), sy - d.y / (yu ? yu : 1));
    }
    evt.Skip();
}

void ZoomPanel::OnEnter(wxMouseEvent& evt) {
    SyncSpaceFromKeyboard();
    evt.Skip();
}
