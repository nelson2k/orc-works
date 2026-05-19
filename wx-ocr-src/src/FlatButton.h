#pragma once

#include <wx/wx.h>
#include <wx/bmpbndl.h>

class FlatButton : public wxPanel {
public:
    FlatButton(wxWindow* parent, const wxString& label,
               const wxBitmapBundle& icon = wxBitmapBundle());

    bool Enable(bool enable = true) override;
    void SetColors(const wxColour& normal, const wxColour& hover, const wxColour& pressed);

private:
    void OnPaint(wxPaintEvent&);
    void OnEnter(wxMouseEvent&);
    void OnLeave(wxMouseEvent&);
    void OnDown(wxMouseEvent&);
    void OnUp(wxMouseEvent&);
    void OnCaptureLost(wxMouseCaptureLostEvent&);

    void EmitClick();

    wxString label_;
    wxBitmapBundle icon_;
    bool hover_ = false;
    bool pressed_ = false;
    wxColour normalCol_   = wxColour(33, 150, 243);
    wxColour hoverCol_    = wxColour(66, 165, 245);
    wxColour pressedCol_  = wxColour(21, 101, 192);
};
