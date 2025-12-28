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
#include "brush.h"
#include "gui.h"
#include "palette_monster.h"
#include "monster_brush.h"
#include "spawn_monster_brush.h"
#include "materials.h"

// ============================================================================
// Monster palette

BEGIN_EVENT_TABLE(MonsterPalettePanel, PalettePanel)
EVT_BUTTON(PALETTE_MONSTER_TILESET_BUTTON, MonsterPalettePanel::OnTilesetButtonClick)
EVT_MENU(wxID_ANY, MonsterPalettePanel::OnTilesetMenuSelect)

EVT_LISTBOX(PALETTE_MONSTER_LISTBOX, MonsterPalettePanel::OnListBoxChange)

EVT_TOGGLEBUTTON(PALETTE_MONSTER_BRUSH_BUTTON, MonsterPalettePanel::OnClickMonsterBrushButton)
EVT_TOGGLEBUTTON(PALETTE_SPAWN_MONSTER_BRUSH_BUTTON, MonsterPalettePanel::OnClickSpawnMonsterBrushButton)

EVT_TEXT(PALETTE_MONSTER_SPAWN_TIME, MonsterPalettePanel::OnChangeSpawnMonsterTime)
EVT_TEXT(PALETTE_MONSTER_SPAWN_SIZE, MonsterPalettePanel::OnChangeSpawnMonsterSize)
END_EVENT_TABLE()

MonsterPalettePanel::MonsterPalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	handling_event(false) {

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Monsters");
	tileset_button = newd wxButton(this, PALETTE_MONSTER_TILESET_BUTTON, "All ▼", wxDefaultPosition, wxDefaultSize, wxBU_LEFT);
	sidesizer->Add(tileset_button, 0, wxEXPAND);

	wxSizer* monsterNameSizer = newd wxStaticBoxSizer(wxHORIZONTAL, this);
	monster_name_text = newd wxTextCtrl(this, PALETTE_MONSTER_SEARCH, "Search name", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	monster_name_text->Bind(wxEVT_SET_FOCUS, &MonsterPalettePanel::OnSetFocus, this);
	monster_name_text->Bind(wxEVT_KILL_FOCUS, &MonsterPalettePanel::OnKillFocus, this);
	monster_name_text->Bind(wxEVT_TEXT_ENTER, &MonsterPalettePanel::OnChangeMonsterNameSearch, this);

	monster_search_button = newd wxButton(this, wxID_ANY, "Search");
	monster_search_button->Bind(wxEVT_BUTTON, &MonsterPalettePanel::OnChangeMonsterNameSearch, this);

	monsterNameSizer->Add(monster_name_text, 2, wxEXPAND);
	monsterNameSizer->Add(monster_search_button, 1, wxEXPAND);
	sidesizer->Add(monsterNameSizer);

	monster_list = newd SortableListBox(this, PALETTE_MONSTER_LISTBOX);
	sidesizer->Add(monster_list, 1, wxEXPAND);
	topsizer->Add(sidesizer, 1, wxEXPAND);

	// Brush selection
	sidesizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Brushes", wxDefaultPosition, wxSize(150, 200)), wxVERTICAL);

	// sidesizer->Add(180, 1, wxEXPAND);

	wxFlexGridSizer* grid = newd wxFlexGridSizer(4, 2, 10, 10);
	grid->AddGrowableCol(1);

	grid->Add(newd wxStaticText(this, wxID_ANY, "Spawntime"));
	monster_spawntime_spin = newd wxTextCtrl(this, PALETTE_MONSTER_SPAWN_TIME, i2ws(g_settings.getInteger(Config::DEFAULT_SPAWN_MONSTER_TIME)), wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
	grid->Add(monster_spawntime_spin, 0, wxEXPAND);

	grid->Add(newd wxStaticText(this, wxID_ANY, "Spawn size"));
	spawn_monster_size_spin = newd wxTextCtrl(this, PALETTE_MONSTER_SPAWN_SIZE, i2ws(1), wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
	grid->Add(spawn_monster_size_spin, 0, wxEXPAND);

	grid->Add(newd wxStaticText(this, wxID_ANY, "Spawn density %"));
	monster_spawndensity_spin = newd wxTextCtrl(this, PALETTE_MONSTER_SPAWN_DENSITY, i2ws(g_settings.getInteger(Config::SPAWN_MONSTER_DENSITY)), wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
	grid->Add(monster_spawndensity_spin, 0, wxEXPAND);

	grid->Add(newd wxStaticText(this, wxID_ANY, "Default weight %"));
	monsterDefaultWeight = newd wxTextCtrl(this, PALETTE_MONSTER_DEFAULT_WEIGHT, i2ws(g_settings.getInteger(Config::MONSTER_DEFAULT_WEIGHT)), wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NUMERIC));
	grid->Add(monsterDefaultWeight, 0, wxEXPAND);

	sidesizer->Add(grid, 0, wxEXPAND);

	monster_brush_button = newd wxToggleButton(this, PALETTE_MONSTER_BRUSH_BUTTON, "Place Monster");
	sidesizer->Add(monster_brush_button, 0, wxEXPAND | wxTOP, 5);

	spawn_monster_brush_button = newd wxToggleButton(this, PALETTE_SPAWN_MONSTER_BRUSH_BUTTON, "Place Spawn");
	sidesizer->Add(spawn_monster_brush_button, 0, wxEXPAND | wxTOP, 5);
	topsizer->Add(sidesizer, 0, wxEXPAND);
	SetSizerAndFit(topsizer);

	OnUpdate();
}

MonsterPalettePanel::~MonsterPalettePanel() {
	monster_name_text->Unbind(wxEVT_SET_FOCUS, &MonsterPalettePanel::OnSetFocus, this);
	monster_name_text->Unbind(wxEVT_KILL_FOCUS, &MonsterPalettePanel::OnKillFocus, this);
	monster_name_text->Unbind(wxEVT_TEXT_ENTER, &MonsterPalettePanel::OnChangeMonsterNameSearch, this);
}

PaletteType MonsterPalettePanel::GetType() const {
	return TILESET_MONSTER;
}

void MonsterPalettePanel::SelectFirstBrush() {
	SelectMonsterBrush();
}

Brush* MonsterPalettePanel::GetSelectedBrush() const {
	if (monster_brush_button->GetValue()) {
		if (monster_list->GetCount() == 0) {
			return nullptr;
		}
		Brush* brush = reinterpret_cast<Brush*>(monster_list->GetClientData(monster_list->GetSelection()));
		if (brush && brush->isMonster()) {
			g_gui.SetSpawnMonsterTime(wxAtoi(monster_spawntime_spin->GetValue()));
			return brush;
		}
	} else if (spawn_monster_brush_button->GetValue()) {
		g_settings.setInteger(Config::CURRENT_SPAWN_MONSTER_RADIUS, wxAtoi(spawn_monster_size_spin->GetValue()));
		g_settings.setInteger(Config::DEFAULT_SPAWN_MONSTER_TIME, wxAtoi(monster_spawntime_spin->GetValue()));
		g_settings.setInteger(Config::SPAWN_MONSTER_DENSITY, wxAtoi(monster_spawndensity_spin->GetValue()));
		std::vector<MonsterBrush*> monsters;
		wxArrayInt selectedIndices;
		monster_list->GetCheckedItems(selectedIndices);
		for (auto index : selectedIndices) {
			Brush* brush = reinterpret_cast<Brush*>(monster_list->GetClientData(index));
			if (brush && brush->isMonster()) {
				monsters.push_back(brush->asMonster());
			}
		}
		g_gui.spawn_brush->setMonsters(monsters);
		return g_gui.spawn_brush;
	}
	return nullptr;
}

bool MonsterPalettePanel::SelectBrush(const Brush* whatbrush) {
	if (!whatbrush) {
		return false;
	}

	if (whatbrush->isMonster()) {
		int current_index = m_selectionIndex;
		if (current_index >= 0 && current_index < (int)m_tilesetData.size()) {
			const TilesetCategory* tsc = m_tilesetData[current_index];
			// Select first house
			for (BrushVector::const_iterator iter = tsc->brushlist.begin(); iter != tsc->brushlist.end(); ++iter) {
				if (*iter == whatbrush) {
					SelectMonster(whatbrush->getName());
					return true;
				}
			}
		}
		// Not in the current display, search the hidden one's
		for (size_t i = 0; i < m_tilesetData.size(); ++i) {
			if (current_index != (int)i) {
				const TilesetCategory* tsc = m_tilesetData[i];
				for (BrushVector::const_iterator iter = tsc->brushlist.begin();
					 iter != tsc->brushlist.end();
					 ++iter) {
					if (*iter == whatbrush) {
						SelectTileset(i);
						SelectMonster(whatbrush->getName());
						return true;
					}
				}
			}
		}
	} else if (whatbrush->isSpawnMonster()) {
		SelectSpawnBrush();
		return true;
	}
	return false;
}

int MonsterPalettePanel::GetSelectedBrushSize() const {
	return wxAtoi(spawn_monster_size_spin->GetValue());
}

void MonsterPalettePanel::OnUpdate() {
	m_tilesetData.clear();
	m_tilesetNames.clear();
	g_materials.createOtherTileset();

	for (TilesetContainer::const_iterator iter = g_materials.tilesets.begin(); iter != g_materials.tilesets.end(); ++iter) {
		const TilesetCategory* tsc = iter->second->getCategory(TILESET_MONSTER);
		if (tsc && tsc->size() > 0) {
			m_tilesetData.push_back(tsc);
			m_tilesetNames.push_back(wxstr(iter->second->name));
		} else if (iter->second->name == "Others") {
			Tileset* ts = const_cast<Tileset*>(iter->second);
			TilesetCategory* rtsc = ts->getCategory(TILESET_MONSTER);
			m_tilesetData.push_back(rtsc);
			m_tilesetNames.push_back(wxstr(ts->name));
		}
	}
	
	// Ensure selection is valid
	if (m_selectionIndex >= (int)m_tilesetData.size()) {
		m_selectionIndex = 0;
	}
	
	SelectTileset(m_selectionIndex);
}

void MonsterPalettePanel::OnUpdateBrushSize(BrushShape shape, int size) {
	return spawn_monster_size_spin->SetValue(i2ws(size));
}

void MonsterPalettePanel::OnSwitchIn() {
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetBrushSize(wxAtoi(spawn_monster_size_spin->GetValue()));
}

// Button Handlers
void MonsterPalettePanel::OnTilesetButtonClick(wxCommandEvent& event) {
	wxMenu menu;
	for (size_t i = 0; i < m_tilesetNames.size(); ++i) {
		auto* item = menu.AppendRadioItem(i, m_tilesetNames[i]);
		if (i == (size_t)m_selectionIndex) {
			item->Check(true);
		}
	}
	const wxSize btnSize = tileset_button->GetSize();
	tileset_button->PopupMenu(&menu, 0, btnSize.GetHeight());
}

void MonsterPalettePanel::OnTilesetMenuSelect(wxCommandEvent& event) {
	int newSelection = event.GetId();
	if (newSelection >= 0 && newSelection < (int)m_tilesetData.size()) {
		SelectTileset(newSelection);
		g_gui.ActivatePalette(GetParentPalette());
		g_gui.SelectBrush();
	}
}

void MonsterPalettePanel::SelectTileset(size_t index) {
	if (m_tilesetData.empty()) {
		monster_list->Clear();
		monster_brush_button->Enable(false);
		tileset_button->SetLabel("All ▼");
		return;
	}

	if (index >= m_tilesetData.size()) {
		index = 0;
	}

	m_selectionIndex = index;
	monster_list->Clear();

	const TilesetCategory* tsc = m_tilesetData[index];
	for (BrushVector::const_iterator iter = tsc->brushlist.begin();
		 iter != tsc->brushlist.end();
		 ++iter) {
		monster_list->Append(wxstr((*iter)->getName()), *iter);
	}
	monster_list->Sort();
	SelectMonster(0);

	tileset_button->SetLabel(m_tilesetNames[index] + " ▼");
}


void MonsterPalettePanel::SelectMonster(size_t index) {
	// Save the old g_settings
	ASSERT(monster_list->GetCount() >= index);

	if (monster_list->GetCount() > 0) {
		monster_list->SetSelection(index);
	}

	SelectMonsterBrush();
}

void MonsterPalettePanel::SelectMonster(std::string name) {
	if (monster_list->GetCount() > 0) {
		if (!monster_list->SetStringSelection(wxstr(name))) {
			monster_list->SetSelection(0);
		}
	}

	SelectMonsterBrush();
}

void MonsterPalettePanel::SelectMonsterBrush() {
	if (monster_list->GetCount() > 0) {
		monster_brush_button->Enable(true);
		monster_brush_button->SetValue(true);
		spawn_monster_brush_button->SetValue(false);
	} else {
		monster_brush_button->Enable(false);
		SelectSpawnBrush();
	}
}

void MonsterPalettePanel::SelectSpawnBrush() {
	// g_gui.house_exit_brush->setHouse(house);
	monster_brush_button->SetValue(false);
	spawn_monster_brush_button->SetValue(true);
}



void MonsterPalettePanel::OnListBoxChange(wxCommandEvent &event) {
	SelectMonster(event.GetSelection());
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void MonsterPalettePanel::OnClickMonsterBrushButton(wxCommandEvent &event) {
	SelectMonsterBrush();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void MonsterPalettePanel::OnClickSpawnMonsterBrushButton(wxCommandEvent &event) {
	SelectSpawnBrush();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void MonsterPalettePanel::OnChangeSpawnMonsterTime(wxCommandEvent &event) {
	g_gui.ActivatePalette(GetParentPalette());
	long val;
	if (event.GetString().ToLong(&val)) {
		g_gui.SetSpawnMonsterTime((int)val);
	}
}

void MonsterPalettePanel::OnChangeSpawnMonsterSize(wxCommandEvent &event) {
	if (!handling_event) {
		handling_event = true;
		g_gui.ActivatePalette(GetParentPalette());
		long val;
		if (event.GetString().ToLong(&val)) {
			g_gui.SetBrushSize((int)val);
		}
		handling_event = false;
	}
}

void MonsterPalettePanel::OnSetFocus(wxFocusEvent &event) {
	g_gui.DisableHotkeys();
	event.Skip();
}

void MonsterPalettePanel::OnKillFocus(wxFocusEvent &event) {
	g_gui.EnableHotkeys();
	event.Skip();
}

void MonsterPalettePanel::OnChangeMonsterNameSearch(wxCommandEvent &event) {
	const auto monsterNameSearch = as_lower_str(monster_name_text->GetValue().ToStdString());

	const auto index = m_selectionIndex;

	if (monsterNameSearch.empty()) {
		SelectTileset(index);
		return;
	}

	monster_list->Clear();
	if (index < 0 || index >= (int)m_tilesetData.size()) return;

	const auto tilesetCategory = m_tilesetData[index];
	for (auto it = tilesetCategory->brushlist.begin(); it != tilesetCategory->brushlist.end(); ++it) {
		const auto monsterName = wxstr((*it)->getName());
		const auto regexPattern = std::regex(monsterNameSearch);
		if (std::regex_search(as_lower_str(monsterName.ToStdString()), regexPattern)) {
			monster_list->Append(monsterName, *it);
		}
	}
	monster_list->Sort();
	Update();
}
