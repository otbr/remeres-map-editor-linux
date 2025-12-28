//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_EAR_CONTROL_H_
#define RME_EAR_CONTROL_H_

#include <wx/wx.h>
#include <vector>
#include <functional>

// Event fired when an ear is clicked
wxDECLARE_EVENT(EVT_EAR_SELECTED, wxCommandEvent);
// Event fired when the add button is clicked
wxDECLARE_EVENT(EVT_EAR_ADD, wxCommandEvent);
// Event fired when an ear is right-clicked to close
wxDECLARE_EVENT(EVT_EAR_CLOSE, wxCommandEvent);

struct EarItem {
    wxString summary;
    bool active;
};

class EarControl : public wxPanel {
public:
    EarControl(wxWindow* parent, wxWindowID id = wxID_ANY);
    ~EarControl() = default;

    void SetItems(const std::vector<wxString>& items, int activeIndex);
    void SetActiveIndex(int index);
    int GetActiveIndex() const { return m_activeIndex; }

    void SetAlignment(bool rightAligned) { m_rightAligned = rightAligned; Refresh(); }

protected:
    void OnPaint(wxPaintEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnEnterWindow(wxMouseEvent& event);
    void OnLeaveWindow(wxMouseEvent& event);

    int GetItemAt(const wxPoint& pos) const;
    void RenderEar(wxDC& dc, const wxRect& rect, const wxString& text, bool active, bool isAddButton = false);
    
    // Helper to calculate tab rectangle
    wxRect GetTabRect(int index, int height) const;

    std::vector<wxString> m_items;
    int m_activeIndex = 0;
    int m_hoverIndex = -1;
    bool m_hoverAdd = false;
    bool m_rightAligned = false;

    // Dimensions
    const int EAR_WIDTH = 22; // Thin vertical strip
    const int EAR_HEIGHT = 80; // Taller tabs
    const int ADD_BUTTON_HEIGHT = 30;

    DECLARE_EVENT_TABLE()
};

#endif
