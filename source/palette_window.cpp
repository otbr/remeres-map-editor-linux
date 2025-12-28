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

#include "palette_window.h"
#include "palette_brushlist.h"
#include "palette_house.h"
#include "palette_monster.h"
#include "palette_npc.h"
#include "palette_waypoints.h"
#include "palette_zones.h"

#include "house_brush.h"
#include "map.h"

// ============================================================================
// Palette window

BEGIN_EVENT_TABLE(PaletteWindow, wxPanel)
EVT_CHOICE(PALETTE_MAIN_CHOICE, PaletteWindow::OnPaletteChoiceChanged)
EVT_CLOSE(PaletteWindow::OnClose)

EVT_KEY_DOWN(PaletteWindow::OnKey)
END_EVENT_TABLE()

PaletteWindow::PaletteWindow(wxWindow* parent, const TilesetContainer &tilesets) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(230, 250)) {
	SetMinSize(wxSize(225, 250));

	// Create separate wxChoice and wxSimplebook for better GTK3 dropdown positioning
	paletteChoice = newd wxChoice(this, PALETTE_MAIN_CHOICE, wxDefaultPosition, wxDefaultSize);
	paletteBook = newd wxSimplebook(this, wxID_ANY);

	// Create and add palettes
	terrainPalette = static_cast<BrushPalettePanel*>(CreateTerrainPalette(paletteBook, tilesets));
	paletteChoice->Append(terrainPalette->GetName());
	paletteBook->AddPage(terrainPalette, terrainPalette->GetName());

	doodadPalette = static_cast<BrushPalettePanel*>(CreateDoodadPalette(paletteBook, tilesets));
	paletteChoice->Append(doodadPalette->GetName());
	paletteBook->AddPage(doodadPalette, doodadPalette->GetName());

	itemPalette = static_cast<BrushPalettePanel*>(CreateItemPalette(paletteBook, tilesets));
	paletteChoice->Append(itemPalette->GetName());
	paletteBook->AddPage(itemPalette, itemPalette->GetName());

	housePalette = static_cast<HousePalettePanel*>(CreateHousePalette(paletteBook, tilesets));
	paletteChoice->Append(housePalette->GetName());
	paletteBook->AddPage(housePalette, housePalette->GetName());

	waypointPalette = static_cast<WaypointPalettePanel*>(CreateWaypointPalette(paletteBook, tilesets));
	paletteChoice->Append(waypointPalette->GetName());
	paletteBook->AddPage(waypointPalette, waypointPalette->GetName());

	zonesPalette = static_cast<ZonesPalettePanel*>(CreateZonesPalette(paletteBook, tilesets));
	paletteChoice->Append(zonesPalette->GetName());
	paletteBook->AddPage(zonesPalette, zonesPalette->GetName());

	monsterPalette = static_cast<MonsterPalettePanel*>(CreateMonsterPalette(paletteBook, tilesets));
	paletteChoice->Append(monsterPalette->GetName());
	paletteBook->AddPage(monsterPalette, monsterPalette->GetName());

	npcPalette = static_cast<NpcPalettePanel*>(CreateNpcPalette(paletteBook, tilesets));
	paletteChoice->Append(npcPalette->GetName());
	paletteBook->AddPage(npcPalette, npcPalette->GetName());

	rawPalette = static_cast<BrushPalettePanel*>(CreateRAWPalette(paletteBook, tilesets));
	paletteChoice->Append(rawPalette->GetName());
	paletteBook->AddPage(rawPalette, rawPalette->GetName());

	// Select first item
	paletteChoice->SetSelection(0);
	paletteBook->SetSelection(0);

	// Setup sizers
	const auto sizer = newd wxBoxSizer(wxVERTICAL);
	sizer->Add(paletteChoice, 0, wxEXPAND | wxALL, 2);
	paletteBook->SetMinSize(wxSize(225, 280));
	sizer->Add(paletteBook, 1, wxEXPAND);
	SetSizer(sizer);

	// Load first page
	LoadCurrentContents();

	Fit();
}

void PaletteWindow::AddBrushToolPanel(PalettePanel* panel, const Config::Key config) {
	const auto toolPanel = newd BrushToolPanel(panel);
	toolPanel->SetToolbarIconSize(g_settings.getBoolean(config));
	panel->AddToolPanel(toolPanel);
}

void PaletteWindow::AddBrushSizePanel(PalettePanel* panel, const Config::Key config) {
	const auto sizePanel = newd BrushSizePanel(panel);
	sizePanel->SetToolbarIconSize(g_settings.getBoolean(config));
	panel->AddToolPanel(sizePanel);
}

PalettePanel* PaletteWindow::CreateTerrainPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd BrushPalettePanel(parent, tilesets, TILESET_TERRAIN);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_TERRAIN_STYLE)));

	AddBrushToolPanel(panel, Config::USE_LARGE_TERRAIN_TOOLBAR);

	AddBrushSizePanel(panel, Config::USE_LARGE_TERRAIN_TOOLBAR);

	return panel;
}

PalettePanel* PaletteWindow::CreateDoodadPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd BrushPalettePanel(parent, tilesets, TILESET_DOODAD);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_DOODAD_STYLE)));

	panel->AddToolPanel(newd BrushThicknessPanel(panel));

	AddBrushSizePanel(panel, Config::USE_LARGE_DOODAD_SIZEBAR);

	return panel;
}

PalettePanel* PaletteWindow::CreateItemPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd BrushPalettePanel(parent, tilesets, TILESET_ITEM);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_ITEM_STYLE)));

	AddBrushSizePanel(panel, Config::USE_LARGE_ITEM_SIZEBAR);

	return panel;
}

PalettePanel* PaletteWindow::CreateHousePalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd HousePalettePanel(parent);

	AddBrushSizePanel(panel, Config::USE_LARGE_HOUSE_SIZEBAR);

	return panel;
}

PalettePanel* PaletteWindow::CreateWaypointPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd WaypointPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateZonesPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd ZonesPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateMonsterPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd MonsterPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateNpcPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd NpcPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateRAWPalette(wxWindow* parent, const TilesetContainer &tilesets) {
	const auto panel = newd BrushPalettePanel(parent, tilesets, TILESET_RAW);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_RAW_STYLE)));

	AddBrushSizePanel(panel, Config::USE_LARGE_RAW_SIZEBAR);

	return panel;
}

bool PaletteWindow::CanSelectHouseBrush(PalettePanel* palette, const Brush* whatBrush) {
	if (!palette || !whatBrush->isHouse()) {
		return false;
	}

	return true;
}

bool PaletteWindow::CanSelectBrush(PalettePanel* palette, const Brush* whatBrush) {
	if (!palette) {
		return false;
	}

	return palette->SelectBrush(whatBrush);
}

void PaletteWindow::ReloadSettings(Map* map) {
	if (terrainPalette) {
		terrainPalette->SetListType(wxstr(g_settings.getString(Config::PALETTE_TERRAIN_STYLE)));
		terrainPalette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR));
	}
	if (doodadPalette) {
		doodadPalette->SetListType(wxstr(g_settings.getString(Config::PALETTE_DOODAD_STYLE)));
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
		itemPalette->SetListType(wxstr(g_settings.getString(Config::PALETTE_ITEM_STYLE)));
		itemPalette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_ITEM_SIZEBAR));
	}
	if (rawPalette) {
		rawPalette->SetListType(wxstr(g_settings.getString(Config::PALETTE_RAW_STYLE)));
		rawPalette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_RAW_SIZEBAR));
	}
	InvalidateContents();
}

void PaletteWindow::LoadCurrentContents() const {
	if (!paletteBook) {
		return;
	}

	const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());

	if (panel == nullptr) {
		return;
	}

	panel->LoadCurrentContents();

	// WASTE OF TIME? IT SEEMS THAT DOESN'T HAVE NO EFFECT.
	// Fit();
	// Refresh();
	// Update();
}

void PaletteWindow::InvalidateContents() {
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

void PaletteWindow::SelectPage(PaletteType id) {
	if (!paletteBook || !paletteChoice) {
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
			paletteChoice->SetSelection(pageIndex);
			g_gui.SelectBrush();
			break;
		}
	}
}

Brush* PaletteWindow::GetSelectedBrush() const {
	if (!paletteBook) {
		return nullptr;
	}

	const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());

	if (panel == nullptr) {
		return nullptr;
	}

	return panel->GetSelectedBrush();
}

int PaletteWindow::GetSelectedBrushSize() const {
	if (!paletteBook) {
		return 0;
	}
	const auto panel = dynamic_cast<PalettePanel*>(paletteBook->GetCurrentPage());

	if (panel == nullptr) {
		return 0;
	}

	return panel->GetSelectedBrushSize();
}

PaletteType PaletteWindow::GetSelectedPage() const {
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

bool PaletteWindow::OnSelectBrush(const Brush* whatBrush, PaletteType primary) {
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

void PaletteWindow::OnPaletteChoiceChanged(wxCommandEvent& event) {
	if (!paletteBook || !paletteChoice) {
		return;
	}

	int newSelection = event.GetSelection();
	int oldSelection = paletteBook->GetSelection();
	
	if (newSelection != oldSelection) {
		OnSwitchingPage(oldSelection, newSelection);
		paletteBook->SetSelection(newSelection);
		g_gui.SelectBrush();
	}
}

void PaletteWindow::OnSwitchingPage(int oldSelection, int newSelection) {
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

void PaletteWindow::OnUpdateBrushSize(BrushShape shape, int size) {
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

void PaletteWindow::OnUpdate(Map* map) {
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

void PaletteWindow::OnKey(wxKeyEvent &event) {
	if (g_gui.GetCurrentTab() != nullptr) {
		g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
	}
}

void PaletteWindow::OnClose(wxCloseEvent &event) {
	if (!event.CanVeto()) {
		// We can't do anything! This sucks!
		// (application is closed, we have to destroy ourselves)
		Destroy();
	} else {
		Show(false);
		event.Veto(true);
	}
}
