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

#include "settings.h"
#include "gui.h"
#include "brush.h"
#include "map_display.h"

#include "palette_view.h"
#include "palette_brushlist.h"
#include "palette_house.h"
#include "palette_monster.h"
#include "palette_npc.h"
#include "palette_waypoints.h"
#include "palette_zones.h"

#include "house_brush.h"
#include "map.h"

// ============================================================================
// Palette View
// ============================================================================

BEGIN_EVENT_TABLE(PaletteView, wxPanel)
EVT_BUTTON(PALETTE_MAIN_BUTTON, PaletteView::OnPaletteButtonClick)
EVT_MENU(wxID_ANY, PaletteView::OnPaletteMenuSelect)
EVT_KEY_DOWN(PaletteView::OnKey)
END_EVENT_TABLE()

wxDEFINE_EVENT(EVT_PALETTE_CONTENT_CHANGED, wxCommandEvent);

PaletteView::PaletteView(wxWindow* parent, const TilesetContainer &tilesets) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize) {
	// SetMinSize(wxSize(225, 250)); // Handled by container

	// Create wxButton + wxMenu to replace native wxChoice and force dropdown direction
	paletteButton = newd wxButton(this, PALETTE_MAIN_BUTTON, "Terrain Palette ▼", wxDefaultPosition, wxDefaultSize, wxBU_LEFT);
	paletteBook = newd wxSimplebook(this, wxID_ANY);

	// Create and add palettes
	terrainPalette = static_cast<BrushPalettePanel*>(CreateTerrainPalette(paletteBook, tilesets));
	paletteBook->AddPage(terrainPalette, terrainPalette->GetName());

	doodadPalette = static_cast<BrushPalettePanel*>(CreateDoodadPalette(paletteBook, tilesets));
	paletteBook->AddPage(doodadPalette, doodadPalette->GetName());

	itemPalette = static_cast<BrushPalettePanel*>(CreateItemPalette(paletteBook, tilesets));
	paletteBook->AddPage(itemPalette, itemPalette->GetName());

	housePalette = static_cast<HousePalettePanel*>(CreateHousePalette(paletteBook, tilesets));
	paletteBook->AddPage(housePalette, housePalette->GetName());

	waypointPalette = static_cast<WaypointPalettePanel*>(CreateWaypointPalette(paletteBook, tilesets));
	paletteBook->AddPage(waypointPalette, waypointPalette->GetName());

	zonesPalette = static_cast<ZonesPalettePanel*>(CreateZonesPalette(paletteBook, tilesets));
	paletteBook->AddPage(zonesPalette, zonesPalette->GetName());

	monsterPalette = static_cast<MonsterPalettePanel*>(CreateMonsterPalette(paletteBook, tilesets));
	paletteBook->AddPage(monsterPalette, monsterPalette->GetName());

	npcPalette = static_cast<NpcPalettePanel*>(CreateNpcPalette(paletteBook, tilesets));
	paletteBook->AddPage(npcPalette, npcPalette->GetName());

	rawPalette = static_cast<BrushPalettePanel*>(CreateRAWPalette(paletteBook, tilesets));
	paletteBook->AddPage(rawPalette, rawPalette->GetName());

	// Select first item
	paletteBook->SetSelection(0);
	paletteButton->SetLabel(terrainPalette->GetName() + " ▼");

	// Setup sizers
	const auto sizer = newd wxBoxSizer(wxVERTICAL);
	sizer->Add(paletteButton, 0, wxEXPAND | wxALL, 2);
	paletteBook->SetMinSize(wxSize(225, 280));
	sizer->Add(paletteBook, 1, wxEXPAND);
	SetSizer(sizer);

	// Load first page
	LoadCurrentContents();

	Fit();
}

void PaletteView::AddBrushToolPanel(PalettePanel* panel, const Config::Key config) {
	const auto toolPanel = newd BrushToolPanel(panel);
	toolPanel->SetToolbarIconSize(g_settings.getBoolean(config));
	panel->AddToolPanel(toolPanel);
}

void PaletteView::AddBrushSizePanel(PalettePanel* panel, const Config::Key config) {
	const auto sizePanel = newd BrushSizePanel(panel);
	sizePanel->SetToolbarIconSize(g_settings.getBoolean(config));
	panel->AddToolPanel(sizePanel);
}

PalettePanel* PaletteView::CreateTerrainPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd BrushPalettePanel(parent, tilesets, TILESET_TERRAIN);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_TERRAIN_STYLE_V2)));

	AddBrushToolPanel(panel, Config::USE_LARGE_TERRAIN_TOOLBAR);

	AddBrushSizePanel(panel, Config::USE_LARGE_TERRAIN_TOOLBAR);

	return panel;
}

PalettePanel* PaletteView::CreateDoodadPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd BrushPalettePanel(parent, tilesets, TILESET_DOODAD);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_DOODAD_STYLE_V2)));

	panel->AddToolPanel(newd BrushThicknessPanel(panel));

	AddBrushSizePanel(panel, Config::USE_LARGE_DOODAD_SIZEBAR);

	return panel;
}

PalettePanel* PaletteView::CreateItemPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd BrushPalettePanel(parent, tilesets, TILESET_ITEM);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_ITEM_STYLE_V2)));

	AddBrushSizePanel(panel, Config::USE_LARGE_ITEM_SIZEBAR);

	return panel;
}

PalettePanel* PaletteView::CreateHousePalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd HousePalettePanel(parent);

	AddBrushSizePanel(panel, Config::USE_LARGE_HOUSE_SIZEBAR);

	return panel;
}

PalettePanel* PaletteView::CreateWaypointPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd WaypointPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteView::CreateZonesPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd ZonesPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteView::CreateMonsterPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd MonsterPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteView::CreateNpcPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd NpcPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteView::CreateRAWPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd BrushPalettePanel(parent, tilesets, TILESET_RAW);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_RAW_STYLE_V2)));

	AddBrushSizePanel(panel, Config::USE_LARGE_RAW_SIZEBAR);

	return panel;
}

bool PaletteView::CanSelectHouseBrush(PalettePanel* palette, const Brush* whatBrush) {
	if (!palette || !whatBrush->isHouse()) {
		return false;
	}

	return true;
}

bool PaletteView::CanSelectBrush(PalettePanel* palette, const Brush* whatBrush) {
	if (!palette) {
		return false;
	}

	return palette->SelectBrush(whatBrush);
}

void PaletteView::ReloadSettings(Map* map) {
	if (terrainPalette) {
		terrainPalette->SetListType(wxstr(g_settings.getString(Config::PALETTE_TERRAIN_STYLE_V2)));
		terrainPalette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR));
	}
	if (doodadPalette) {
		doodadPalette->SetListType(wxstr(g_settings.getString(Config::PALETTE_DOODAD_STYLE_V2)));
		doodadPalette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_DOODAD_SIZEBAR));
	}
	if (housePalette) {
		housePalette->SetMap(map);
		housePalette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_HOUSE_SIZEBAR));
	}
	if (waypointPalette) {
		waypointPalette->SetMap(map);
	}
	if (zonesPalette) {
		zonesPalette->SetMap(map);
	}
	if (itemPalette) {
		itemPalette->SetListType(wxstr(g_settings.getString(Config::PALETTE_ITEM_STYLE_V2)));
		itemPalette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_ITEM_SIZEBAR));
	}
	if (rawPalette) {
		rawPalette->SetListType(wxstr(g_settings.getString(Config::PALETTE_RAW_STYLE_V2)));
		rawPalette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_RAW_SIZEBAR));
	}
	InvalidateContents();
}

void PaletteView::LoadCurrentContents() const {
	if (!paletteBook) {
		return;
	}

	const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());

	if (panel == nullptr) {
		return;
	}

	panel->LoadCurrentContents();
}

void PaletteView::InvalidateContents() {
	if (!paletteBook) {
		return;
	}
	for (auto pageIndex = 0; pageIndex < paletteBook->GetPageCount(); ++pageIndex) {
		const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetPage(pageIndex));
		if (panel != nullptr) {
			panel->InvalidateContents();
		}
	}
	LoadCurrentContents();
	if (monsterPalette) {
		monsterPalette->OnUpdate();
	}
	if (npcPalette) {
		npcPalette->OnUpdate();
	}
	if (housePalette) {
		housePalette->OnUpdate();
	}
	if (waypointPalette) {
		waypointPalette->OnUpdate();
	}
	if (zonesPalette) {
		zonesPalette->OnUpdate();
	}
}

void PaletteView::SelectPage(PaletteType id) {
	if (!paletteBook || !paletteButton) {
		return;
	}
	if (id == GetSelectedPage()) {
		return;
	}

	for (auto pageIndex = 0; pageIndex < paletteBook->GetPageCount(); ++pageIndex) {
		const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetPage(pageIndex));
		if (panel == nullptr) {
			return;
		}

		if (panel->GetType() == id) {
			int oldSelection = paletteBook->GetSelection();
			OnSwitchingPage(oldSelection, pageIndex);
			paletteBook->SetSelection(pageIndex);
			paletteButton->SetLabel(panel->GetName() + " ▼");
			g_gui.SelectBrush();
			break;
		}
	}
}

Brush* PaletteView::GetSelectedBrush() const {
	if (!paletteBook) {
		return nullptr;
	}

	const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());

	if (panel == nullptr) {
		return nullptr;
	}

	return panel->GetSelectedBrush();
}

int PaletteView::GetSelectedBrushSize() const {
	if (!paletteBook) {
		return 0;
	}
	const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());

	if (panel == nullptr) {
		return 0;
	}

	return panel->GetSelectedBrushSize();
}

PaletteType PaletteView::GetSelectedPage() const {
	if (!paletteBook) {
		return TILESET_UNKNOWN;
	}
	const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());

	ASSERT(panel);
	if (panel == nullptr) {
		return TILESET_UNKNOWN;
	}

	return panel->GetType();
}

bool PaletteView::OnSelectBrush(const Brush* whatBrush, PaletteType primary) {
	if (!paletteBook || !whatBrush) {
		return false;
	}

	if (CanSelectHouseBrush(housePalette, whatBrush)) {
		housePalette->SelectBrush(whatBrush);
		SelectPage(TILESET_HOUSE);
		return true;
	}

	switch (primary) {
		case TILESET_TERRAIN: {
			// This is already searched first
			break;
		}
		case TILESET_DOODAD: {
			// Ok, search doodad before terrain
			if (CanSelectBrush(doodadPalette, whatBrush)) {
				SelectPage(TILESET_DOODAD);
				return true;
			}
			break;
		}
		case TILESET_ITEM: {
			if (CanSelectBrush(itemPalette, whatBrush)) {
				SelectPage(TILESET_ITEM);
				return true;
			}
			break;
		}
		case TILESET_MONSTER: {
			if (CanSelectBrush(monsterPalette, whatBrush)) {
				SelectPage(TILESET_MONSTER);
				return true;
			}
			break;
		}
		case TILESET_NPC: {
			if (CanSelectBrush(npcPalette, whatBrush)) {
				SelectPage(TILESET_NPC);
				return true;
			}
			break;
		}
		case TILESET_RAW: {
			if (CanSelectBrush(rawPalette, whatBrush)) {
				SelectPage(TILESET_RAW);
				return true;
			}
			break;
		}
		default:
			break;
	}

	// Test if it's a terrain brush
	if (CanSelectBrush(terrainPalette, whatBrush)) {
		SelectPage(TILESET_TERRAIN);
		return true;
	}

	// Test if it's a doodad brush
	if (primary != TILESET_DOODAD && CanSelectBrush(doodadPalette, whatBrush)) {
		SelectPage(TILESET_DOODAD);
		return true;
	}

	// Test if it's an item brush
	if (primary != TILESET_ITEM && CanSelectBrush(itemPalette, whatBrush)) {
		SelectPage(TILESET_ITEM);
		return true;
	}

	// Test if it's a monster brush
	if (primary != TILESET_MONSTER && CanSelectBrush(monsterPalette, whatBrush)) {
		SelectPage(TILESET_MONSTER);
		return true;
	}

	// Test if it's a npc brush
	if (primary != TILESET_NPC && CanSelectBrush(npcPalette, whatBrush)) {
		SelectPage(TILESET_NPC);
		return true;
	}

	// Test if it's a raw brush
	if (primary != TILESET_RAW && CanSelectBrush(rawPalette, whatBrush)) {
		SelectPage(TILESET_RAW);
		return true;
	}

	return false;
}

// Custom handler to show menu below the button
void PaletteView::OnPaletteButtonClick(wxCommandEvent& event) {
	if (!paletteBook) return;

	wxMenu menu;
	for (size_t i = 0; i < paletteBook->GetPageCount(); ++i) {
		const auto page = dynamic_cast<PalettePanel*>(paletteBook->GetPage(i));
		if (page) {
			auto* item = menu.AppendRadioItem(i, page->GetName());
			if (i == (size_t)paletteBook->GetSelection()) {
				item->Check(true);
			}
		}
	}

	// Calculate position: bottom-left of the button
	const wxSize btnSize = paletteButton->GetSize();
	paletteButton->PopupMenu(&menu, 0, btnSize.GetHeight());
}

void PaletteView::OnPaletteMenuSelect(wxCommandEvent& event) {
	if (!paletteBook || !paletteButton) return;

	int newSelection = event.GetId();
	int oldSelection = paletteBook->GetSelection();

	if (newSelection >= 0 && newSelection < (int)paletteBook->GetPageCount()) {
		if (newSelection != oldSelection) {
			OnSwitchingPage(oldSelection, newSelection);
			paletteBook->SetSelection(newSelection);
			
			// Update button label
			const auto page = dynamic_cast<PalettePanel*>(paletteBook->GetPage(newSelection));
			if (page) {
				paletteButton->SetLabel(page->GetName() + " ▼");
			}

			g_gui.SelectBrush();

			wxCommandEvent evt(EVT_PALETTE_CONTENT_CHANGED, GetId());
			evt.SetEventObject(this);
			ProcessEvent(evt);
		}
	}
}

void PaletteView::OnSwitchingPage(int oldSelection, int newSelection) {
	if (!paletteBook) {
		return;
	}

	if (oldSelection >= 0 && oldSelection < static_cast<int>(paletteBook->GetPageCount())) {
		const auto oldPage = paletteBook->GetPage(oldSelection);
		const auto oldPanel = dynamic_cast<PalettePanel*>(oldPage);
		if (oldPanel) {
			oldPanel->OnSwitchOut();
		}
	}

	if (newSelection >= 0 && newSelection < static_cast<int>(paletteBook->GetPageCount())) {
		const auto selectedPage = paletteBook->GetPage(newSelection);
		const auto selectedPanel = dynamic_cast<PalettePanel*>(selectedPage);
		if (selectedPanel) {
			selectedPanel->OnSwitchIn();
		}
	}
}

void PaletteView::OnUpdateBrushSize(BrushShape shape, int size) {
	if (!paletteBook) {
		return;
	}
	const auto page = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());

	ASSERT(page);

	if (page == nullptr) {
		return;
	}

	page->OnUpdateBrushSize(shape, size);
}

void PaletteView::OnUpdate(Map* map) {
	if (monsterPalette) {
		monsterPalette->OnUpdate();
	}
	if (npcPalette) {
		npcPalette->OnUpdate();
	}
	if (housePalette) {
		housePalette->SetMap(map);
	}
	if (waypointPalette) {
		waypointPalette->SetMap(map);
		waypointPalette->OnUpdate();
	}
	if (zonesPalette) {
		zonesPalette->SetMap(map);
		zonesPalette->OnUpdate();
	}
}

void PaletteView::OnKey(wxKeyEvent &event) {
	if (g_gui.GetCurrentTab() != nullptr) {
		g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
	}
}

wxString PaletteView::GetTabSummary() const {
	if (!paletteBook) {
		return "Empty";
	}
	const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());
	if (panel) {
		return panel->GetContentSummary();
	}
	return "Unk";
}

void PaletteView::OnActivateView() {
	if (paletteBook) {
		const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());
		if (panel) {
			panel->OnSwitchIn();
		}
	}
}

void PaletteView::OnDeactivateView() {
	if (paletteBook) {
		const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());
		if (panel) {
			panel->OnSwitchOut();
		}
	}
}
