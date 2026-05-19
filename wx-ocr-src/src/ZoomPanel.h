#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>

class ZoomPanel : public wxScrolled<wxPanel> {
public:
    ZoomPanel(wxWindow* parent);

    void SetImage(const wxImage& img);
    bool HasImage() const { return image_.IsOk(); }

    void SetCtrlDown(bool v) { ctrlDown_ = v; }
    void SetSpaceDown(bool v);

private:
    void OnPaint(wxPaintEvent&);
    void OnMouseWheel(wxMouseEvent&);
    void OnMouseDown(wxMouseEvent&);
    void OnMouseUp(wxMouseEvent&);
    void OnMouseMotion(wxMouseEvent&);

    void UpdateVirtualSize();
    void RebuildBitmap();

    wxImage image_;
    wxBitmap bitmap_;
    double scale_ = 1.0;
    bool ctrlDown_ = false;
    bool spaceDown_ = false;
    bool dragging_ = false;
    wxPoint dragLast_;
};
