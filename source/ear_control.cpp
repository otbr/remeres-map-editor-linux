//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "ear_control.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

wxDEFINE_EVENT(EVT_EAR_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_EAR_ADD, wxCommandEvent);

BEGIN_EVENT_TABLE(EarControl, wxPanel)
    EVT_PAINT(EarControl::OnPaint)
    EVT_LEFT_DOWN(EarControl::OnMouseEvent)
    EVT_MOTION(EarControl::OnMouseEvent)
    EVT_LEAVE_WINDOW(EarControl::OnLeaveWindow)
    EVT_ERASE_BACKGROUND(EarControl::OnEraseBackground)
END_EVENT_TABLE()

// ... (Previous event table remains same)

// ... (Previous event table)

// ... (Previous event table)

// ... (Previous event table)

EarControl::EarControl(wxWindow* parent, wxWindowID id) 
    : wxPanel(parent, id, wxDefaultPosition, wxSize(-1, 28)) // Fixed Height 28
{
    SetMinSize(wxSize(-1, 28));
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void EarControl::SetItems(const std::vector<wxString>& items, int activeIndex) {
    m_items = items;
    m_activeIndex = activeIndex;
    Refresh();
}

void EarControl::SetActiveIndex(int index) {
    if (m_activeIndex != index) {
        m_activeIndex = index;
        Refresh();
    }
}

void EarControl::OnEraseBackground(wxEraseEvent& event) {
    // reduce flicker
}

void EarControl::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    
    // Clear background
    wxColour bg = wxColour(40, 40, 40); // Fixed dark theme header color
    dc.SetBackground(wxBrush(bg));
    dc.Clear();

    int width = GetClientSize().GetWidth();
    int height = GetClientSize().GetHeight();
    
    int itemCount = m_items.size();
    
    // Draw Items
    for (size_t i = 0; i < m_items.size(); ++i) {
        wxRect rect = GetTabRect(i, height);
        RenderEar(dc, rect, m_items[i], (int)i == m_activeIndex);
    }

    // Draw Add Button
    wxRect addRect = GetTabRect(-2, height); // -2 code for ADD
    RenderEar(dc, addRect, "+", false, true);
    
    // Draw Bottom Line for entire control to match border
    wxColour borderCol = wxColour(60, 60, 60);
    dc.SetPen(wxPen(borderCol));
    dc.DrawLine(0, height - 1, width, height - 1);
}

wxRect EarControl::GetTabRect(int index, int height) const {
    int startX = 0;
    wxClientDC dc(const_cast<EarControl*>(this));
    dc.SetFont(GetFont());

    int clientWidth = GetClientSize().GetWidth();
    
    // Pre-calculate all widths
    int addBtnWidth = 30;
    std::vector<int> itemWidths;
    int totalItemsWidth = 0;
    
    for (size_t i = 0; i < m_items.size(); ++i) {
        wxCoord w, h;
        dc.GetTextExtent(m_items[i], &w, &h);
        int itemW = w + 20; // Padding
        if (itemW < 50) itemW = 50; 
        itemWidths.push_back(itemW);
        totalItemsWidth += itemW;
    }

    if (!m_rightAligned) {
        // Left Alignment: Add Button First, Then Tabs
        // [Add] [Tab1] [Tab2] ...
        
        int currentX = 0;
        
        // Add Button at Left Extreme 
        if (index == -2) {
            return wxRect(0, 0, addBtnWidth, height);
        }
        
        currentX += addBtnWidth;
        
        for (size_t i = 0; i < m_items.size(); ++i) {
            if ((int)i == index) {
                return wxRect(currentX, 0, itemWidths[i], height);
            }
            currentX += itemWidths[i];
        }

    } else {
        // Right Alignment: Add Button Last (Extreme Right), Tabs Before it
        // ... [Tab2] [Tab1] [Add]
        
        // Start from Right edge
        int currentX = clientWidth;
        
        // Add Button at Extreme Right
        if (index == -2) { 
            return wxRect(clientWidth - addBtnWidth, 0, addBtnWidth, height);
        }
        
        currentX -= addBtnWidth;
        
        // Tabs rendered in reverse order from the right? 
        // Or standard order but pushed to right?
        // User said: "place the one on the right side to start on the right side"
        // Usually right aligned tabs: [Tab1] [Tab2] [Add] (Right aligned block)
        // OR: [Tab1] ... [TabN] [Add]
        
        // Let's stack them from the right.
        // If we want [Tab1] [Tab2] [Add] (Right Aligned), then:
        // X of Add = W - AddW
        // X of TabN = W - AddW - TabNW
        // ...
        
        // Wait, "Extreme Corners".
        // Left: [Add] [Tab1] ...
        // Right: ... [Tab1] [Add]
        
        // To maintain order 1..N visually reading Left->Right:
        // We need to calculate total width and shift startX.
        
        int totalGroupWidth = totalItemsWidth;
        // Start X for first tab:
        int groupStartX = clientWidth - addBtnWidth - totalGroupWidth;
        
        int tabX = groupStartX;
         for (size_t i = 0; i < m_items.size(); ++i) {
            if ((int)i == index) {
                return wxRect(tabX, 0, itemWidths[i], height);
            }
            tabX += itemWidths[i];
        }
    }
    
    return wxRect(0,0,0,0);
}

void EarControl::RenderEar(wxDC& dc, const wxRect& rect, const wxString& text, bool active, bool isAddButton) {
    wxColour bgCol = GetParent()->GetBackgroundColour();
    wxColour textCol = wxColour(200, 200, 200);
    
    if (active) {
        // Active Tab: Slightly lighter background or just text highlight
        // Pro Style: Top Border Highlight
        wxColour activeBg = bgCol.ChangeLightness(120);
        dc.SetBrush(wxBrush(activeBg));
        dc.SetPen(wxPen(activeBg));
        dc.DrawRectangle(rect);
        
        textCol = *wxWHITE;
    } else {
        // Transparent/BG
        dc.SetBrush(wxBrush(bgCol));
        dc.SetPen(wxPen(bgCol));
        dc.DrawRectangle(rect);
    }

    if (isAddButton) {
        if (m_hoverAdd) {
             dc.SetBrush(wxBrush(bgCol.ChangeLightness(130)));
             dc.SetPen(wxPen(bgCol.ChangeLightness(130)));
             dc.DrawRectangle(rect);
        }
    }

    // Draw Text
    wxFont font = GetFont();
    if (active || isAddButton) {
        font.SetWeight(wxFONTWEIGHT_BOLD);
    }
    dc.SetFont(font);
    dc.SetTextForeground(textCol);
    
    wxCoord textW, textH;
    dc.GetTextExtent(text, &textW, &textH);
    
    dc.DrawText(text, rect.GetX() + (rect.GetWidth() - textW) / 2, rect.GetY() + (rect.GetHeight() - textH) / 2);

    // Draw Separator (Vertical line)
    // We want separators between items and between items and Add button.
    // Left Align: [Add] | [Tab1] | [Tab2]
    // Right Align: [Tab1] | [Tab2] | [Add]
    
    // Logic: Draw separator on the RIGHT of the item/button, UNLESS it's the last item on the right edge.
    // For Right Align, Add Button is last.
    
    wxColour sepCol = wxColour(80, 80, 80);
    dc.SetPen(wxPen(sepCol));
    int pad = 6;
    
    // Rule: Always draw separator on the side "facing inside"?
    // Or just simple Right separator for everyone except the very last thing on screen?
    // Actually, distinct separation for Add Button is nice.
    
    // Let's just draw Right Separator for everyone.
    // If rect.GetRight() near Screen Width, skip.
    
    int clientW = GetClientSize().GetWidth();
    if (rect.GetRight() < clientW - 2) {
         dc.DrawLine(rect.GetRight(), rect.GetTop() + pad, rect.GetRight(), rect.GetBottom() - pad);
    } 
    
    // For Left Layout, maybe we want separator on the Left of items? [Add] | [Tab]
    // If we draw Right separator on Add button, it works: [Add] | 
    // Then [Tab1] | 
    
    // For Right Layout: [Tab1] | [Tab2] | [Add]
    // Drawing Right separator works here too.
    
    // Active Indicator Line (Bottom)
    if (active) {
        dc.SetPen(wxPen(wxColour(100, 200, 255), 2)); // Cyan-ish highlight
        dc.DrawLine(rect.GetLeft(), rect.GetBottom()-1, rect.GetRight(), rect.GetBottom()-1);
    }
}

int EarControl::GetItemAt(const wxPoint& pos) const {
    if (pos.y < 0 || pos.y > GetClientSize().GetHeight()) return -1;
    
    // Iterate to find which rect contains pos.x
    int height = GetClientSize().GetHeight();
    
    // Check items
    for (size_t i = 0; i < m_items.size(); ++i) {
        wxRect r = GetTabRect(i, height);
        if (r.Contains(pos)) return (int)i;
    }
    
    // Check Add Button
    wxRect rAdd = GetTabRect(-2, height);
    if (rAdd.Contains(pos)) return -2;
    
    return -1;
}

void EarControl::OnMouseEvent(wxMouseEvent& event) {
    if (event.LeftDown()) {
        int index = GetItemAt(event.GetPosition());
        if (index >= 0) {
            SetActiveIndex(index);
            wxCommandEvent evt(EVT_EAR_SELECTED, GetId());
            evt.SetEventObject(this);
            evt.SetInt(index);
            ProcessEvent(evt);
        } else if (index == -2) {
            wxCommandEvent evt(EVT_EAR_ADD, GetId());
            evt.SetEventObject(this);
            ProcessEvent(evt);
        }
    } else if (event.Moving()) {
         int index = GetItemAt(event.GetPosition());
         bool changed = false;
         
         if (index == -2 && !m_hoverAdd) {
             m_hoverAdd = true;
             changed = true;
         } else if (index != -2 && m_hoverAdd) {
             m_hoverAdd = false;
             changed = true;
         }
         
         if (changed) Refresh();
    }
}

void EarControl::OnLeaveWindow(wxMouseEvent& event) {
    if (m_hoverAdd) {
        m_hoverAdd = false;
        Refresh();
    }
}
