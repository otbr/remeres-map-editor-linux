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

#ifndef RME_PALETTE_H_
#define RME_PALETTE_H_

#include "palette_common.h"
#include <vector>

class PaletteView;
class EarControl;

// Refactored PaletteWindow class that acts as a container for multiple "PaletteViews"
// If multiple views are present, it shows the "EarControl" to switch between them.
class PaletteWindow : public wxPanel {
public:
	PaletteWindow(wxWindow* parent, const TilesetContainer &tilesets);
	~PaletteWindow();

	// Container Interface
	// Adds a new view to this window (creates a new ear)
	void AddView();
	// Removes the currently active view
	void RemoveCurrentView();

	// Proxy Interface (Forwards to Active View)
	void ReloadSettings(Map* from);
	void InvalidateContents();
	void LoadCurrentContents() const;
	void SelectPage(PaletteType palette);
	Brush* GetSelectedBrush() const;
	int GetSelectedBrushSize() const;
	PaletteType GetSelectedPage() const;
	
	// Custom Event handlers (Forwards to Active View)
	virtual bool OnSelectBrush(const Brush* whatBrush, PaletteType primary = TILESET_UNKNOWN);
	virtual void OnUpdateBrushSize(BrushShape shape, int size);
	virtual void OnUpdate(Map* map);

	// Event Handlers for Container
	void OnEarSelected(wxCommandEvent& event);
	void OnEarAdd(wxCommandEvent& event);
    void OnPaletteContentChanged(wxCommandEvent& event);
	void OnClose(wxCloseEvent &);
    void OnSize(wxSizeEvent &event); // To handle layout of ears

    // Temporary accessor to getting internal views if needed
    PaletteView* GetActiveView() const;
    
    EarControl* GetEarControl() const { return m_earControl; }

protected:
    void UpdateLayout();
    void UpdateEars();

	EarControl* m_earControl = nullptr;
    std::vector<PaletteView*> m_views;
    PaletteView* m_activeView = nullptr;
    const TilesetContainer& m_tilesets;

    wxBoxSizer* m_mainSizer = nullptr;

	DECLARE_EVENT_TABLE()
};

#endif
