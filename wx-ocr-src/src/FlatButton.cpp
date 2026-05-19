#include "FlatButton.h"

#include <wx/dcbuffer.h>

namespace {
const wxColour kText(255, 255, 255);
}

FlatButton::FlatButton(wxWindow* parent, const wxString& label,
                       const wxBitmapBundle& icon)
    : wxPanel(parent, wxID_ANY), label_(label), icon_(icon) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    wxFont f = GetFont();
    f.MakeBold();
    f.SetPointSize(f.GetPointSize() + 1);
    SetFont(f);

    wxClientDC dc(this);
    dc.SetFont(f);
    wxSize textSize = dc.GetTextExtent(label_);
    int iconW = icon_.IsOk() ? icon_.GetDefaultSize().GetWidth() : 0;
    int padX = FromDIP(14);
    int gap = iconW > 0 ? FromDIP(6) : 0;
    int width = padX + iconW + gap + textSize.GetWidth() + padX;
    int height = FromDIP(36);
    SetMinSize(wxSize(width, height));

    Bind(wxEVT_PAINT, &FlatButton::OnPaint, this);
    Bind(wxEVT_ENTER_WINDOW, &FlatButton::OnEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &FlatButton::OnLeave, this);
    Bind(wxEVT_LEFT_DOWN, &FlatButton::OnDown, this);
    Bind(wxEVT_LEFT_UP, &FlatButton::OnUp, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &FlatButton::OnCaptureLost, this);
}

void FlatButton::SetColors(const wxColour& normal, const wxColour& hover, const wxColour& pressed) {
    normalCol_ = normal;
    hoverCol_ = hover;
    pressedCol_ = pressed;
    Refresh(false);
}

bool FlatButton::Enable(bool enable) {
    bool changed = wxPanel::Enable(enable);
    if (changed) {
        if (!enable) { hover_ = false; pressed_ = false; }
        Refresh(false);
    }
    return changed;
}

void FlatButton::OnEnter(wxMouseEvent& e) {
    if (IsEnabled()) { hover_ = true; Refresh(false); }
    e.Skip();
}

void FlatButton::OnLeave(wxMouseEvent& e) {
    hover_ = false;
    Refresh(false);
    e.Skip();
}

void FlatButton::OnDown(wxMouseEvent& e) {
    if (!IsEnabled()) return;
    pressed_ = true;
    if (!HasCapture()) CaptureMouse();
    Refresh(false);
    e.Skip();
}

void FlatButton::OnUp(wxMouseEvent& e) {
    if (HasCapture()) ReleaseMouse();
    bool wasPressed = pressed_;
    pressed_ = false;
    Refresh(false);
    if (wasPressed && IsEnabled()) {
        wxRect r = GetClientRect();
        if (r.Contains(e.GetPosition())) EmitClick();
    }
    e.Skip();
}

void FlatButton::OnCaptureLost(wxMouseCaptureLostEvent&) {
    pressed_ = false;
    Refresh(false);
}

void FlatButton::EmitClick() {
    wxCommandEvent evt(wxEVT_BUTTON, GetId());
    evt.SetEventObject(this);
    ProcessWindowEvent(evt);
}

void FlatButton::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    wxSize sz = GetClientSize();

    wxColour bg;
    if (pressed_)    bg = pressedCol_;
    else if (hover_) bg = hoverCol_;
    else             bg = normalCol_;

    dc.SetBrush(wxBrush(bg));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());

    dc.SetFont(GetFont());
    dc.SetTextForeground(kText);

    int padX = FromDIP(14);
    int gap = FromDIP(6);
    int x = padX;

    if (icon_.IsOk()) {
        wxBitmap bmp = icon_.GetBitmap(icon_.GetDefaultSize());
        int by = (sz.GetHeight() - bmp.GetHeight()) / 2;
        dc.DrawBitmap(bmp, x, by, true);
        x += bmp.GetWidth() + gap;
    }

    wxSize textSize = dc.GetTextExtent(label_);
    int ty = (sz.GetHeight() - textSize.GetHeight()) / 2;
    dc.DrawText(label_, x, ty);
}
