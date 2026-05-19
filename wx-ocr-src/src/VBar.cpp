#include "VBar.h"

#include <wx/dcbuffer.h>

VBar::VBar(wxWindow* parent, const wxString& name, const wxColour& fill)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(34, 120)),
      name_(name), fillCol_(fill) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &VBar::OnPaint, this);
    Bind(wxEVT_SIZE, &VBar::OnSize, this);
}

void VBar::Set(double frac, const wxString& label) {
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;
    frac_ = frac;
    value_ = label;
    Refresh(false);
}

void VBar::OnSize(wxSizeEvent& evt) {
    Refresh(false);
    evt.Skip();
}

void VBar::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    wxSize sz = GetClientSize();

    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();

    wxFont font = GetFont();
    font.SetPointSize(8);
    dc.SetFont(font);

    wxColour fg = GetForegroundColour();
    dc.SetTextForeground(fg);

    wxSize valueExt = dc.GetTextExtent(value_);
    wxSize nameExt = dc.GetTextExtent(name_);

    int pad = 4;
    int valueY = 0;
    int nameY = sz.GetHeight() - nameExt.GetHeight();

    dc.DrawText(value_, (sz.GetWidth() - valueExt.GetWidth()) / 2, valueY);
    dc.DrawText(name_, (sz.GetWidth() - nameExt.GetWidth()) / 2, nameY);

    int trackTop = valueExt.GetHeight() + pad;
    int trackBottom = nameY - pad;
    int trackH = trackBottom - trackTop;
    if (trackH < 0) trackH = 0;

    int barW = sz.GetWidth() - 8;
    if (barW < 6) barW = 6;
    int barX = (sz.GetWidth() - barW) / 2;

    wxColour trackCol(60, 60, 60);
    dc.SetBrush(wxBrush(trackCol));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(barX, trackTop, barW, trackH);

    int fillH = (int)((double)trackH * frac_);
    if (fillH > 0) {
        dc.SetBrush(wxBrush(fillCol_));
        dc.DrawRectangle(barX, trackTop + (trackH - fillH), barW, fillH);
    }
}
