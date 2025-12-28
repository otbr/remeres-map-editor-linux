//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "palette_window.h"
#include "palette_view.h"
#include "ear_control.h"

// ============================================================================
// Palette Window (Container)
// ============================================================================

BEGIN_EVENT_TABLE(PaletteWindow, wxPanel)
    EVT_CLOSE(PaletteWindow::OnClose)
END_EVENT_TABLE()

PaletteWindow::PaletteWindow(wxWindow* parent, const TilesetContainer &tilesets) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(230, 250)),
    m_tilesets(tilesets) 
{
    // Create Ear Control (Exogenous - Parented to Main Window/Root)
    // We assume 'parent' is the dock window or something managed by AUI. 
    // To be safe for AUI Top Dock, let's parent it to the same parent as this window initially?
    // Actually, for AUI floating/docking, it usually needs to be child of fluid frame.
    // GUI::CreatePalette will handle parenting if needed, or we just create it here with 'parent' 
    // and let AUI reparent it when AddPane is called? 
    // wxAuiManager::AddPane usually handles reparenting if necessary, or expects child of managed window.
    // Let's keep 'parent' for now.
    
    m_earControl = newd EarControl(parent); // Parent is root frame usually
    m_earControl->Hide(); 

    // Bind custom events directly to the control since it's not our child anymore
    m_earControl->Bind(EVT_EAR_SELECTED, &PaletteWindow::OnEarSelected, this);
    m_earControl->Bind(EVT_EAR_ADD, &PaletteWindow::OnEarAdd, this);
    
    // Internal event
    Bind(EVT_PALETTE_CONTENT_CHANGED, &PaletteWindow::OnPaletteContentChanged, this);

	SetMinSize(wxSize(225, 250));

	// Create Layout: VBox [PaletteView] (EarControl is purely external now)
    m_mainSizer = newd wxBoxSizer(wxVERTICAL);
    
    // Create initial view
    AddView();

	SetSizer(m_mainSizer);
	Fit();
}

PaletteWindow::~PaletteWindow() {
    // Views are children, will be deleted by wx
}

void PaletteWindow::AddView() {
    PaletteView* view = newd PaletteView(this, m_tilesets);
    m_views.push_back(view);

    if (m_views.size() == 1) {
        // First view
        // Sizer: [View (0)]
        m_mainSizer->Add(view, 1, wxEXPAND);
        m_activeView = view;
    } else {
        // New view is hidden until we switch
        view->Hide(); 
        
        // Switch to new view immediately
        if (m_activeView) m_activeView->Hide();
        m_mainSizer->Detach(m_activeView);
        
        // Add new view to bottom
        m_mainSizer->Add(view, 1, wxEXPAND);
        
        view->Show();
        m_activeView = view;
        
        // m_earControl->Show(); // Managed externally by GUI/AUI
    }
    
    UpdateEars();
    Layout();
}

void PaletteWindow::UpdateEars() {
    if (!m_earControl) return;

    if (m_views.size() > 1) {
        std::vector<wxString> items;
        int activeIdx = 0;
        for (size_t i = 0; i < m_views.size(); ++i) {
            items.push_back(m_views[i]->GetTabSummary());
            if (m_views[i] == m_activeView) activeIdx = i;
        }
        m_earControl->SetItems(items, activeIdx);
        m_earControl->Show();
    } else {
        m_earControl->Hide();
    }
}

void PaletteWindow::OnEarSelected(wxCommandEvent& event) {
    int index = event.GetInt();
    if (index >= 0 && index < (int)m_views.size()) {
        PaletteView* newView = m_views[index];
        if (newView != m_activeView) {
            
            // Swap in sizer
            m_mainSizer->Detach(m_activeView);
            m_activeView->Hide();
            m_activeView->OnDeactivateView();

            // Add to end (below tabs)
            m_mainSizer->Add(newView, 1, wxEXPAND);
            newView->Show();
            newView->OnActivateView();
            m_activeView = newView;
            
            Layout();
            Refresh();
        }
    }
}

void PaletteWindow::OnEarAdd(wxCommandEvent& event) {
    AddView();
    // Auto switch is handled in AddView
}

void PaletteWindow::OnPaletteContentChanged(wxCommandEvent& event) {
    UpdateEars();
    event.Skip();
}

// Proxies
void PaletteWindow::ReloadSettings(Map* from) {
    if (m_activeView) m_activeView->ReloadSettings(from);
}

void PaletteWindow::InvalidateContents() {
    for (auto* view : m_views) {
        view->InvalidateContents();
    }
}

void PaletteWindow::LoadCurrentContents() const {
    if (m_activeView) m_activeView->LoadCurrentContents();
}

void PaletteWindow::SelectPage(PaletteType palette) {
    if (m_activeView) m_activeView->SelectPage(palette);
}

Brush* PaletteWindow::GetSelectedBrush() const {
    if (m_activeView) return m_activeView->GetSelectedBrush();
    return nullptr;
}

int PaletteWindow::GetSelectedBrushSize() const {
    if (m_activeView) return m_activeView->GetSelectedBrushSize();
    return 0;
}

PaletteType PaletteWindow::GetSelectedPage() const {
    if (m_activeView) return m_activeView->GetSelectedPage();
    return TILESET_UNKNOWN;
}

bool PaletteWindow::OnSelectBrush(const Brush* whatBrush, PaletteType primary) {
    // Special logic: might need to search ALL views if not found in current?
    // Requirement said: "Clicar em uma 'orelha' troca o conteúdo central da palette para os dados daquela instância."
    // But when selecting a brush from the map (pipette), should we switch tabs?
    // Probably yes.
    
    if (m_activeView && m_activeView->OnSelectBrush(whatBrush, primary)) {
        return true;
    }
    
    // Search other views
    for (size_t i = 0; i < m_views.size(); ++i) {
        if (m_views[i] == m_activeView) continue;
        
        if (m_views[i]->OnSelectBrush(whatBrush, primary)) {
            // Found it! Switch to this view.
            wxCommandEvent evt(EVT_EAR_SELECTED);
            evt.SetInt(i);
            OnEarSelected(evt);
            
            // Update ears UI to reflect change
             UpdateEars();
             
             return true;
        }
    }
    
    return false;
}

void PaletteWindow::OnUpdateBrushSize(BrushShape shape, int size) {
    if (m_activeView) m_activeView->OnUpdateBrushSize(shape, size);
}

void PaletteWindow::OnUpdate(Map* map) {
    for (auto* view : m_views) {
        view->OnUpdate(map);
    }
}

void PaletteWindow::OnClose(wxCloseEvent &event) {
    if (!event.CanVeto()) {
        Destroy();
    } else {
        Show(false);
        event.Veto(true);
    }
}

PaletteView* PaletteWindow::GetActiveView() const {
    return m_activeView;
}
