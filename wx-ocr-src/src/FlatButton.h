#pragma once

#include <wx/wx.h>
#include <wx/bmpbndl.h>

class FlatButton : public wxPanel {
public:
    FlatButton(wxWindow* parent, const wxString& label,
               const wxBitmapBundle& icon = wxBitmapBundle());

    bool Enable(bool enable = true) override;

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
};
