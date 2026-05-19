#include "ZoomPanel.h"

#include <wx/dcbuffer.h>

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

void ZoomPanel::SetSpaceDown(bool v) {
    spaceDown_ = v;
    if (v) SetCursor(wxCursor(wxCURSOR_HAND));
    else   SetCursor(wxNullCursor);
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
    if (spaceDown_) {
        dragging_ = true;
        dragLast_ = evt.GetPosition();
        CaptureMouse();
    }
    evt.Skip();
}

void ZoomPanel::OnMouseUp(wxMouseEvent& evt) {
    if (dragging_) {
        dragging_ = false;
        if (HasCapture()) ReleaseMouse();
    }
    evt.Skip();
}

void ZoomPanel::OnMouseMotion(wxMouseEvent& evt) {
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
