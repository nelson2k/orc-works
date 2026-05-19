#pragma once

#include <wx/wx.h>

class VBar : public wxPanel {
public:
    VBar(wxWindow* parent, const wxString& name, const wxColour& fill);
    void Set(double frac, const wxString& label);

private:
    void OnPaint(wxPaintEvent&);
    void OnSize(wxSizeEvent&);

    wxString name_;
    wxColour fillCol_;
    double frac_ = 0.0;
    wxString value_ = "--";
};
