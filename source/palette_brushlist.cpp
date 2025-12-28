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

#include "palette_brushlist.h"
#include "gui.h"
#include "brush.h"
#include "add_tileset_window.h"
#include "add_item_window.h"
#include "materials.h"

// ============================================================================
// Brush Palette Panel
// A common class for terrain/doodad/item/raw palette

BEGIN_EVENT_TABLE(BrushPalettePanel, PalettePanel)
EVT_BUTTON(wxID_ADD, BrushPalettePanel::OnClickAddItemToTileset)
EVT_BUTTON(wxID_NEW, BrushPalettePanel::OnClickAddTileset)
EVT_BUTTON(wxID_FORWARD, BrushPalettePanel::OnNextPage)
EVT_BUTTON(wxID_BACKWARD, BrushPalettePanel::OnPreviousPage)
EVT_COMBOBOX(wxID_ANY, BrushPalettePanel::OnCategoryListChanged)
END_EVENT_TABLE()

BrushPalettePanel::BrushPalettePanel(wxWindow* parent, const TilesetContainer &tilesets, TilesetCategoryType category, wxWindowID id) :
	PalettePanel(parent, id),
	paletteType(category) {

	// Create main layout: Tileset list on top (fixed height), content below
	const auto tsSizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Tileset");
	
	// Category dropdown (restored wxComboBox for dropdown behavior)
	m_categoryCombo = newd wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY);
	tsSizer->Add(m_categoryCombo, 0, wxEXPAND | wxBOTTOM, 5);
	
	// Page container - will hold the current BrushPanel
	m_pageContainer = newd wxPanel(this, wxID_ANY);
	m_pageContainer->SetSizer(newd wxBoxSizer(wxVERTICAL));
	tsSizer->Add(m_pageContainer, 1, wxEXPAND);
	
	sizer->Add(tsSizer, 1, wxEXPAND);

	if (g_settings.getBoolean(Config::SHOW_TILESET_EDITOR)) {
		AddTilesetEditor();
	}

	sizer->Add(pageInfoSizer);

	// Populate categories and create pages
	for (auto it = tilesets.begin(); it != tilesets.end(); ++it) {
		const auto tilesetCategory = it->second->getCategory(category);
		if (tilesetCategory && !tilesetCategory->brushlist.empty()) {
			// Add to combo
			m_categoryCombo->Append(wxstr(it->second->name));
			
			// Create the panel (hidden initially)
			const auto panel = newd BrushPanel(m_pageContainer, tilesetCategory);
			panel->Hide();
			m_pages.push_back(panel);
		}
	}
	
	// Select first page if available
	if (!m_pages.empty()) {
		m_categoryCombo->SetSelection(0);
		ChangeSelection(0);
	}

	SetSizerAndFit(sizer);
}

BrushPalettePanel::~BrushPalettePanel() {
	if (currentPageCtrl) {
		currentPageCtrl->Unbind(wxEVT_SET_FOCUS, &BrushPalettePanel::OnSetFocus, this);
		currentPageCtrl->Unbind(wxEVT_KILL_FOCUS, &BrushPalettePanel::OnKillFocus, this);
		currentPageCtrl->Unbind(wxEVT_TEXT_ENTER, &BrushPalettePanel::OnSetPage, this);
	}
}

void BrushPalettePanel::OnSetFocus(wxFocusEvent &event) {
	g_gui.DisableHotkeys();
	event.Skip();
}

void BrushPalettePanel::OnKillFocus(wxFocusEvent &event) {
	g_gui.EnableHotkeys();
	event.Skip();
}

void BrushPalettePanel::RemovePagination() {
	pageInfoSizer->ShowItems(false);
	pageInfoSizer->Clear();
}

void BrushPalettePanel::AddPagination() {
	RemovePagination();

	const auto buttonsSize = wxSize(55, 25);
	const auto middleElementsSize = wxSize(35, 25);

	nextPageButton = newd wxButton(this, wxID_FORWARD, "->", wxDefaultPosition, buttonsSize);
	currentPageCtrl = newd wxTextCtrl(this, wxID_ANY, "1", wxDefaultPosition, middleElementsSize, wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
	pageInfo = newd wxStaticText(this, wxID_ANY, "/x", wxPoint(0, 5), middleElementsSize);
	previousPageButton = newd wxButton(this, wxID_BACKWARD, "<-", wxDefaultPosition, buttonsSize);

	currentPageCtrl->Bind(wxEVT_SET_FOCUS, &BrushPalettePanel::OnSetFocus, this);
	currentPageCtrl->Bind(wxEVT_KILL_FOCUS, &BrushPalettePanel::OnKillFocus, this);
	currentPageCtrl->Bind(wxEVT_TEXT_ENTER, &BrushPalettePanel::OnSetPage, this);

	pageInfoSizer->Add(previousPageButton, wxEXPAND);
	pageInfoSizer->AddSpacer(15);
	pageInfoSizer->Add(currentPageCtrl);
	pageInfoSizer->AddSpacer(5);
	pageInfoSizer->Add(pageInfo);
	pageInfoSizer->AddSpacer(15);
	pageInfoSizer->Add(nextPageButton, wxEXPAND);
}

void BrushPalettePanel::AddTilesetEditor() {
	const auto tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	const auto buttonAddTileset = newd wxButton(this, wxID_NEW, "Add new Tileset");
	tmpsizer->Add(buttonAddTileset, wxSizerFlags(0).Center());

	const auto buttonAddItemToTileset = newd wxButton(this, wxID_ADD, "Add new Item");
	tmpsizer->Add(buttonAddItemToTileset, wxSizerFlags(0).Center());

	sizer->Add(tmpsizer, 0, wxCENTER, 10);
}

// === Page Management Methods ===

BrushPanel* BrushPalettePanel::GetCurrentPage() const {
	if (m_currentPageIndex >= 0 && m_currentPageIndex < static_cast<int>(m_pages.size())) {
		return m_pages[m_currentPageIndex];
	}
	return nullptr;
}

int BrushPalettePanel::GetSelection() const {
	return m_currentPageIndex;
}

wxString BrushPalettePanel::GetPageText(int index) const {
	if (m_categoryCombo && index >= 0 && index < static_cast<int>(m_categoryCombo->GetCount())) {
		return m_categoryCombo->GetString(index);
	}
	return wxEmptyString;
}

void BrushPalettePanel::ChangeSelection(int index) {
	if (index < 0 || index >= static_cast<int>(m_pages.size())) {
		return;
	}
	
	// Hide old page
	if (m_currentPageIndex >= 0 && m_currentPageIndex < static_cast<int>(m_pages.size())) {
		BrushPanel* oldPanel = m_pages[m_currentPageIndex];
		if (oldPanel) {
			oldPanel->OnSwitchOut();
			oldPanel->Hide();
			// Remember brush selection
			for (const auto palettePanel : tool_bars) {
				const auto brush = palettePanel->GetSelectedBrush();
				if (brush) {
					rememberedBrushes[oldPanel] = brush;
				}
			}
		}
	}
	
	// Show new page
	m_currentPageIndex = index;
	BrushPanel* newPanel = m_pages[m_currentPageIndex];
	if (newPanel) {
		// Add to container sizer if not already added
		wxSizer* containerSizer = m_pageContainer->GetSizer();
		if (containerSizer->GetItemCount() == 0 || containerSizer->GetItem(newPanel) == nullptr) {
			containerSizer->Add(newPanel, 1, wxEXPAND);
		}
		newPanel->Show();
		newPanel->OnSwitchIn();
		
		// Restore remembered brush
		for (const auto palettePanel : tool_bars) {
			palettePanel->SelectBrush(rememberedBrushes[newPanel]);
		}
		
		m_pageContainer->Layout();
	}
	
	// Update list selection
	if (m_categoryCombo && m_categoryCombo->GetSelection() != index) {
		m_categoryCombo->SetSelection(index);
	}
}

// === Interface Methods ===

void BrushPalettePanel::InvalidateContents() {
	for (auto panel : m_pages) {
		if (panel) {
			panel->InvalidateContents();
		}
	}
	PalettePanel::InvalidateContents();
}

void BrushPalettePanel::LoadCurrentContents() {
	BrushPanel* panel = GetCurrentPage();
	if (panel) {
		panel->OnSwitchIn();
	}
	PalettePanel::LoadCurrentContents();
}

void BrushPalettePanel::LoadAllContents() {
	for (auto panel : m_pages) {
		if (panel) {
			panel->LoadContents();
		}
	}
	PalettePanel::LoadAllContents();
}

PaletteType BrushPalettePanel::GetType() const {
	return paletteType;
}

BrushListType BrushPalettePanel::GetListType() const {
	if (m_pages.empty()) {
		return BRUSHLIST_LISTBOX;
	}
	return m_pages[0]->GetListType();
}

void BrushPalettePanel::SetListType(BrushListType newListType) {
	if (m_pages.empty()) {
		return;
	}

	RemovePagination();

	for (auto panel : m_pages) {
		if (panel) {
			panel->SetListType(newListType);
		}
	}
}

void BrushPalettePanel::SetListType(const wxString &newListType) {
	if (m_pages.empty()) {
		return;
	}

	const auto it = listTypeMap.find(newListType);
	if (it == listTypeMap.end()) {
		return;
	}

	SetListType(it->second);
}

Brush* BrushPalettePanel::GetSelectedBrush() const {
	BrushPanel* panel = GetCurrentPage();
	if (!panel) {
		return nullptr;
	}
	
	// Check tool bars first
	for (const auto &palettePanel : tool_bars) {
		Brush* brush = palettePanel->GetSelectedBrush();
		if (brush) {
			return brush;
		}
	}
	return panel->GetSelectedBrush();
}

void BrushPalettePanel::SelectFirstBrush() {
	BrushPanel* panel = GetCurrentPage();
	if (panel) {
		panel->SelectFirstBrush();
	}
}

bool BrushPalettePanel::SelectBrush(const Brush* whatBrush) {
	BrushPanel* panel = GetCurrentPage();
	if (!panel) {
		return false;
	}

	if (panel->SelectBrush(whatBrush)) {
		for (const auto palettePanel : tool_bars) {
			palettePanel->SelectBrush(nullptr);
		}
		return true;
	}

	for (const auto palettePanel : tool_bars) {
		if (palettePanel->SelectBrush(whatBrush)) {
			panel->SelectBrush(nullptr);
			return true;
		}
	}

	// Search other pages
	for (int pageIndex = 0; pageIndex < static_cast<int>(m_pages.size()); ++pageIndex) {
		if (pageIndex == m_currentPageIndex) {
			continue;
		}

		BrushPanel* otherPanel = m_pages[pageIndex];
		if (otherPanel && otherPanel->SelectBrush(whatBrush)) {
			ChangeSelection(pageIndex);
			for (const auto palettePanel : tool_bars) {
				palettePanel->SelectBrush(nullptr);
			}
			return true;
		}
	}
	return false;
}

// === Event Handlers ===

void BrushPalettePanel::OnCategoryListChanged(wxCommandEvent &event) {
	int selection = m_categoryCombo->GetSelection();
	if (selection != wxNOT_FOUND && selection != m_currentPageIndex) {
		ChangeSelection(selection);
		g_gui.ActivatePalette(GetParentPalette());
		g_gui.SelectBrush();
	}
}

void BrushPalettePanel::OnSwitchIn() {
	LoadCurrentContents();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetBrushSizeInternal(last_brush_size);
	OnUpdateBrushSize(g_gui.GetBrushShape(), last_brush_size);
}

void BrushPalettePanel::OnClickAddTileset(wxCommandEvent &WXUNUSED(event)) {
	if (m_pages.empty()) {
		return;
	}

	const auto window = newd AddTilesetWindow(g_gui.root, paletteType);
	const auto result = window->ShowModal();
	window->Destroy();

	if (result != 0) {
		g_gui.DestroyPalettes();
		g_gui.NewPalette();
	}
}

void BrushPalettePanel::OnClickAddItemToTileset(wxCommandEvent &WXUNUSED(event)) {
	if (m_pages.empty() || m_currentPageIndex < 0) {
		return;
	}
	const auto &tilesetName = GetPageText(m_currentPageIndex).ToStdString();

	const auto it = g_materials.tilesets.find(tilesetName);

	if (it != g_materials.tilesets.end()) {
		const auto window = newd AddItemWindow(g_gui.root, paletteType, it->second);
		const auto result = window->ShowModal();
		window->Destroy();

		if (result != 0) {
			g_gui.RebuildPalettes();
		}
	}
}

void BrushPalettePanel::OnPageUpdate(BrushBoxInterface* brushbox, int page) {
	// No-op: Pagination removed in favor of scrolling
}

void BrushPalettePanel::OnSetPage(wxCommandEvent &WXUNUSED(event)) {
	// No-op: Pagination removed in favor of scrolling
}

void BrushPalettePanel::OnNextPage(wxCommandEvent &WXUNUSED(event)) {
	// No-op: Pagination removed in favor of scrolling
}

void BrushPalettePanel::OnPreviousPage(wxCommandEvent &WXUNUSED(event)) {
	// No-op: Pagination removed in favor of scrolling
}

void BrushPalettePanel::EnableNextPage(bool enable /* = true*/) {
	if (!nextPageButton) {
		return;
	}
	nextPageButton->Enable(enable);
}

void BrushPalettePanel::EnablePreviousPage(bool enable /* = true*/) {
	if (!previousPageButton) {
		return;
	}
	previousPageButton->Enable(enable);
}

void BrushPalettePanel::SetPageInfo(const wxString &text) {
	if (!pageInfo) {
		return;
	}
	pageInfo->SetLabelText(text);
}

void BrushPalettePanel::SetCurrentPage(const wxString &value) {
	if (!currentPageCtrl) {
		return;
	}
	currentPageCtrl->SetValue(value);
}

// ============================================================================
// Brush Panel
// A container of brush buttons

BEGIN_EVENT_TABLE(BrushPanel, wxPanel)
// Listbox style
EVT_LISTBOX(wxID_ANY, BrushPanel::OnClickListBoxRow)
END_EVENT_TABLE()

BrushPanel::BrushPanel(wxWindow* parent, const TilesetCategory* tileset) :
	wxPanel(parent, wxID_ANY), tileset(tileset) {
	SetSizerAndFit(sizer);
}

void BrushPanel::AssignTileset(const TilesetCategory* newTileset) {
	if (newTileset != tileset) {
		InvalidateContents();
		tileset = newTileset;
	}
}

BrushListType BrushPanel::GetListType() const {
	return listType;
}

void BrushPanel::SetListType(BrushListType newListType) {
	if (listType != newListType) {
		InvalidateContents();
		listType = newListType;
	}
}

void BrushPanel::SetListType(const wxString &newListType) {
	const auto it = listTypeMap.find(newListType);
	if (it != listTypeMap.end()) {
		SetListType(it->second);
	}
}

void BrushPanel::InvalidateContents() {
	sizer->Clear(true);
	loaded = false;
	brushbox = nullptr;
}

void BrushPanel::LoadContents() {
	if (loaded) {
		return;
	}
	loaded = true;
	ASSERT(tileset != nullptr);
	switch (listType) {
		case BRUSHLIST_LARGE_ICONS:
			brushbox = newd BrushIconBox(this, tileset, RENDER_SIZE_32x32);
			break;
		case BRUSHLIST_SMALL_ICONS:
			brushbox = newd BrushIconBox(this, tileset, RENDER_SIZE_16x16);
			break;
		case BRUSHLIST_LISTBOX:
			brushbox = newd BrushListBox(this, tileset);
			break;
		default:
			break;
	}
	ASSERT(brushbox != nullptr);
	sizer->Add(brushbox->GetSelfWindow(), 1, wxEXPAND);
	Fit();
	brushbox->SelectFirstBrush();
}

void BrushPanel::SelectFirstBrush() {
	if (loaded) {
		ASSERT(brushbox != nullptr);
		brushbox->SelectFirstBrush();
	}
}

Brush* BrushPanel::GetSelectedBrush() const {
	if (loaded) {
		ASSERT(brushbox != nullptr);
		return brushbox->GetSelectedBrush();
	}

	if (tileset && tileset->size() > 0) {
		return tileset->brushlist[0];
	}
	return nullptr;
}

bool BrushPanel::SelectBrush(const Brush* whatBrush) {
	if (loaded) {
		// std::cout << loaded << std::endl;
		// std::cout << brushbox << std::endl;
		ASSERT(brushbox != nullptr);
		return brushbox->SelectBrush(whatBrush);
	}

	for (const auto brush : tileset->brushlist) {
		if (brush == whatBrush) {
			LoadContents();
			return brushbox->SelectBrush(whatBrush);
		}
	}
	return false;
}

void BrushPanel::OnSwitchIn() {
	LoadContents();
}

void BrushPanel::OnSwitchOut() {
	////
}

void BrushPanel::OnClickListBoxRow(wxCommandEvent &event) {
	ASSERT(tileset->getType() >= TILESET_UNKNOWN && tileset->getType() <= TILESET_HOUSE);
	// We just notify the GUI of the action, it will take care of everything else
	ASSERT(brushbox);
	const auto index = event.GetSelection();

	if (const auto paletteWindow = g_gui.GetParentWindowByType<PaletteWindow*>(this); paletteWindow != nullptr) {
		g_gui.ActivatePalette(paletteWindow);
	}

	g_gui.SelectBrush(tileset->brushlist[index], tileset->getType());
}

BrushBoxInterface* BrushPanel::GetBrushBox() const {
	return brushbox;
}

// ============================================================================
// BrushIconBox

// ============================================================================
// BrushIconBox

BEGIN_EVENT_TABLE(BrushIconBox, wxScrolledWindow)
EVT_PAINT(BrushIconBox::OnPaint)
EVT_SIZE(BrushIconBox::OnSize)
EVT_LEFT_UP(BrushIconBox::OnLeftUp)
EVT_MOTION(BrushIconBox::OnMotion)
EVT_LEAVE_WINDOW(BrushIconBox::OnLeave)
END_EVENT_TABLE()

BrushIconBox::BrushIconBox(wxWindow* parent, const TilesetCategory* tileset, RenderSize rsz) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxBORDER_NONE),
	BrushBoxInterface(tileset),
	iconSize(rsz) {
	ASSERT(tileset->getType() >= TILESET_UNKNOWN && tileset->getType() <= TILESET_HOUSE);
	
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetBackgroundColour(*wxWHITE);

	m_cellWidth = (iconSize == RENDER_SIZE_32x32) ? 36 : 20; // 32+4 padding
	m_cellHeight = (iconSize == RENDER_SIZE_32x32) ? 36 : 20;

	// Initial layout calculation
	RecalculateLayout();
}

void BrushIconBox::RecalculateLayout() {
	int width, height;
	GetClientSize(&width, &height);

	if (width < m_cellWidth) width = m_cellWidth; // prevent div by zero
	
	m_cols = width / m_cellWidth;
	if (m_cols < 1) m_cols = 1;

	size_t count = tileset->brushlist.size();
	m_rows = (count + m_cols - 1) / m_cols;

	SetVirtualSize(m_cols * m_cellWidth, m_rows * m_cellHeight);
	SetScrollRate(0, m_cellHeight);
}

void BrushIconBox::OnSize(wxSizeEvent& event) {
	RecalculateLayout();
	event.Skip();
	Refresh();
}

void BrushIconBox::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	DoPrepareDC(dc);

	dc.Clear();

	if (!tileset || tileset->brushlist.empty()) return;

	// Optimization: Determine visible range
	int startX, startY, noUnitsX, noUnitsY;
	GetViewStart(&startX, &startY);
	int clientW, clientH;
	GetClientSize(&clientW, &clientH);

	// Calculate visible rows
	// ViewStart is in scroll units (m_cellHeight)
	int startRow = startY; 
	int endRow = startRow + (clientH / m_cellHeight) + 2; // +buffer

	if (startRow < 0) startRow = 0;
	if (endRow > m_rows) endRow = m_rows;

	int startIndex = startRow * m_cols;
	int endIndex = endRow * m_cols;
	if (endIndex > (int)tileset->brushlist.size()) endIndex = tileset->brushlist.size();

	for (int i = startIndex; i < endIndex; ++i) {
		int col = i % m_cols;
		int row = i / m_cols;
		
		int x = col * m_cellWidth;
		int y = row * m_cellHeight;

		wxRect cellRect(x, y, m_cellWidth, m_cellHeight);
		
		// Draw Selection/Hover bg
		if (i == m_selectedIndex) {
			dc.SetBrush(wxBrush(wxColor(180, 210, 255)));
			dc.SetPen(wxPen(wxColor(50, 100, 200)));
			dc.DrawRectangle(cellRect);
		} else if (i == m_hoverIndex) {
			dc.SetBrush(wxBrush(wxColor(220, 230, 255)));
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.DrawRectangle(cellRect);
		}

		// Draw Sprite
		const auto brush = tileset->brushlist[i];
		if (const auto sprite = g_gui.gfx.getSprite(brush->getLookID()); sprite) {
			// Center sprite in cell
			int spriteSize = (iconSize == RENDER_SIZE_32x32) ? 32 : 16;
			int offX = (m_cellWidth - spriteSize) / 2;
			int offY = (m_cellHeight - spriteSize) / 2;
			
			// Use proper enum for size if available, or just raw numbers? 
			// BrushListBox uses SPRITE_SIZE_32x32. We trust it maps to ints or enum correctly.
			// Actually DrawTo takes SpriteSize enum for implementation but maybe int is fine?
			// BrushListBox calls `sprite->DrawTo(&dc, SPRITE_SIZE_32x32, ...)`
			// We should match that.
			SpriteSize sz = (iconSize == RENDER_SIZE_32x32) ? SPRITE_SIZE_32x32 : SPRITE_SIZE_32x32; // Assuming small also draws at 32 or scale?
			// Actually raw palette is usually 32x32. If small is requested, we might need SPRITE_SIZE_32x32 scaled?
			// Let's stick to 32x32 for now as that's the main request.
			
			sprite->DrawTo(&dc, sz, x + offX, y + offY, spriteSize, spriteSize);
		}
	}
}

void BrushIconBox::OnMotion(wxMouseEvent& event) {
	wxPoint pos = event.GetPosition();
	// Adjust for scrolling
	CalcUnscrolledPosition(pos.x, pos.y, &pos.x, &pos.y);

	int index = HitTest(pos);
	if (index != m_hoverIndex) {
		m_hoverIndex = index;
		Refresh();
		
		if (index >= 0 && index < (int)tileset->brushlist.size()) {
			SetToolTip(tileset->brushlist[index]->getName());
		} else {
			UnsetToolTip();
		}
	}
}

void BrushIconBox::OnLeave(wxMouseEvent& event) {
	if (m_hoverIndex != -1) {
		m_hoverIndex = -1;
		Refresh();
	}
}

void BrushIconBox::OnLeftUp(wxMouseEvent& event) {
	wxPoint pos = event.GetPosition();
	CalcUnscrolledPosition(pos.x, pos.y, &pos.x, &pos.y);
	
	int index = HitTest(pos);
	if (index >= 0 && index < (int)tileset->brushlist.size()) {
		Select(index);
		
		// Notify GUI
		const auto brush = tileset->brushlist[index];
		if (const auto paletteWindow = g_gui.GetParentWindowByType<PaletteWindow*>(this); paletteWindow) {
			g_gui.ActivatePalette(paletteWindow);
		}
		g_gui.SelectBrush(brush, tileset->getType());
	}
}

int BrushIconBox::HitTest(const wxPoint& pt) const {
	int col = pt.x / m_cellWidth;
	int row = pt.y / m_cellHeight;
	
	if (col >= m_cols) return -1;
	
	int index = row * m_cols + col;
	if (index >= 0 && index < (int)tileset->brushlist.size()) {
		return index;
	}
	return -1;
}

void BrushIconBox::Select(int index) {
	m_selectedIndex = index;
	Refresh();
	
	// Ensure visible? logic handled by EnsureVisible if called, or we could do it here
}

// Interface implementations
bool BrushIconBox::LoadContentByPage(int page) {
	// Not used in scroll mode
	return true;
}

bool BrushIconBox::LoadAllContent() {
	RecalculateLayout();
	Refresh();
	return true;
}

void BrushIconBox::SelectFirstBrush() {
	if (tileset && !tileset->brushlist.empty()) {
		Select(0);
	}
}

Brush* BrushIconBox::GetSelectedBrush() const {
	if (m_selectedIndex >= 0 && m_selectedIndex < (int)tileset->brushlist.size()) {
		return tileset->brushlist[m_selectedIndex];
	}
	return nullptr;
}

bool BrushIconBox::SelectPaginatedBrush(const Brush* whatBrush, BrushPalettePanel* brushPalettePanel) {
	return SelectBrush(whatBrush);
}

bool BrushIconBox::SelectBrush(const Brush* whatBrush) {
	if (!whatBrush) {
		m_selectedIndex = -1;
		Refresh();
		return false;
	}
	
	for (size_t i = 0; i < tileset->brushlist.size(); ++i) {
		if (tileset->brushlist[i] == whatBrush) {
			Select(static_cast<int>(i));
			// Ensure visible logic:
			int row = i / m_cols;
			int y = row * m_cellHeight;
			int startX, startY;
			GetViewStart(&startX, &startY);
			// StartY is in scroll units (m_cellHeight)
			int visibleRows = GetClientSize().y / m_cellHeight;
			if (row < startY || row >= startY + visibleRows) {
				Scroll(0, row);
			}
			return true;
		}
	}
	return false;
}

bool BrushIconBox::NextPage() { return false; }
bool BrushIconBox::SetPage(int page) { return false; }
bool BrushIconBox::PreviousPage() { return false; }

// Old methods stubs
void BrushIconBox::EnsureVisible(const BrushButton* brushButto) {
	// No-op
}

void BrushIconBox::Deselect() { }

// ============================================================================
// BrushListBox

BEGIN_EVENT_TABLE(BrushListBox, wxVListBox)
EVT_KEY_DOWN(BrushListBox::OnKey)
END_EVENT_TABLE()

BrushListBox::BrushListBox(wxWindow* parent, const TilesetCategory* tileset) :
	wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE),
	BrushBoxInterface(tileset) {
	SetItemCount(tileset->size());
}

void BrushListBox::SelectFirstBrush() {
	SetSelection(0);
	wxWindow::ScrollLines(-1);
}

Brush* BrushListBox::GetSelectedBrush() const {
	if (!tileset) {
		return nullptr;
	}

	if (const auto index = GetSelection(); index != wxNOT_FOUND) {
		return tileset->brushlist[index];
	} else if (tileset->size() > 0) {
		return tileset->brushlist[0];
	}
	return nullptr;
}

bool BrushListBox::SelectPaginatedBrush(const Brush* whatBrush, BrushPalettePanel* brushPalettePanel) noexcept {
	return false;
}

bool BrushListBox::SelectBrush(const Brush* whatBrush) {
	for (auto index = 0; index < tileset->brushlist.size(); ++index) {
		if (tileset->brushlist[index] == whatBrush) {
			SetSelection(index);
			return true;
		}
	}
	return false;
}

void BrushListBox::OnDrawItem(wxDC &dc, const wxRect &rect, size_t index) const {
	ASSERT(index < tileset->size());
	if (const auto sprite = g_gui.gfx.getSprite(tileset->brushlist[index]->getLookID()); sprite) {
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	}
	if (IsSelected(index)) {
		if (HasFocus()) {
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		} else {
			dc.SetTextForeground(wxColor(0x00, 0x00, 0xFF));
		}
	} else {
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}
	dc.DrawText(wxstr(tileset->brushlist[index]->getName()), rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord BrushListBox::OnMeasureItem(size_t index) const {
	return 32;
}

void BrushListBox::OnKey(wxKeyEvent &event) {
	switch (event.GetKeyCode()) {
		case WXK_UP:
		case WXK_DOWN:
		case WXK_LEFT:
		case WXK_RIGHT:
			if (g_settings.getInteger(Config::LISTBOX_EATS_ALL_EVENTS)) {
				case WXK_PAGEUP:
				case WXK_PAGEDOWN:
				case WXK_HOME:
				case WXK_END:
					event.Skip(true);
			} else {
				[[fallthrough]];
				default:
					if (g_gui.GetCurrentTab() != nullptr) {
						g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
					}
			}
	}
}
