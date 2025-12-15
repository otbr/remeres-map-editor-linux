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
#include "border_editor_window.h"
#include "browse_tile_window.h"
#include "find_item_window.h"
#include "common_windows.h"
#include "graphics.h"
#include "gui.h"
#include "artprovider.h"
#include "items.h"
#include "brush.h"
#include "ground_brush.h"
#include "client_assets.h"
#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/dcbuffer.h>
#include <wx/filename.h>
#include <wx/filepicker.h>
#include <wx/settings.h>
#include <pugixml.hpp>

#define BORDER_GRID_SIZE 32
#define BORDER_PREVIEW_SIZE 192
#define BORDER_GRID_CELL_SIZE 32
#define ID_BORDER_GRID_SELECT wxID_HIGHEST + 1
#define ID_GROUND_ITEM_LIST wxID_HIGHEST + 2

// Utility functions for edge string/position conversion
BorderEdgePosition edgeStringToPosition(const std::string& edgeStr) {
    if (edgeStr == "n") return EDGE_N;
    if (edgeStr == "e") return EDGE_E;
    if (edgeStr == "s") return EDGE_S;
    if (edgeStr == "w") return EDGE_W;
    if (edgeStr == "cnw") return EDGE_CNW;
    if (edgeStr == "cne") return EDGE_CNE;
    if (edgeStr == "cse") return EDGE_CSE;
    if (edgeStr == "csw") return EDGE_CSW;
    if (edgeStr == "dnw") return EDGE_DNW;
    if (edgeStr == "dne") return EDGE_DNE;
    if (edgeStr == "dse") return EDGE_DSE;
    if (edgeStr == "dsw") return EDGE_DSW;
    return EDGE_NONE;
}

std::string edgePositionToString(BorderEdgePosition pos) {
    switch (pos) {
        case EDGE_N: return "n";
        case EDGE_E: return "e";
        case EDGE_S: return "s";
        case EDGE_W: return "w";
        case EDGE_CNW: return "cnw";
        case EDGE_CNE: return "cne";
        case EDGE_CSE: return "cse";
        case EDGE_CSW: return "csw";
        case EDGE_DNW: return "dnw";
        case EDGE_DNE: return "dne";
        case EDGE_DSE: return "dse";
        case EDGE_DSW: return "dsw";
        default: return "";
    }
}

// Add a helper function at the top of the file to get item ID from brush
uint16_t GetItemIDFromBrush(Brush* brush) {
    if (!brush) {
        wxLogDebug("GetItemIDFromBrush: Brush is null");

        return 0;
    }
    
    uint16_t id = 0;
    
    wxLogDebug("GetItemIDFromBrush: Checking brush type: %s", wxString(brush->getName()).c_str());

    
    // First prioritize RAW brush - this is the most direct approach
    if (brush->isRaw()) {
        RAWBrush* rawBrush = brush->asRaw();
        if (rawBrush) {
            id = rawBrush->getItemID();
            wxLogDebug("GetItemIDFromBrush: Found RAW brush ID: %d", id);

            if (id > 0) {
                return id;
            }
        }
    } 
    
    // Then try getID which sometimes works directly
    id = brush->getID();
    if (id > 0) {
        wxLogDebug("GetItemIDFromBrush: Got ID from brush->getID(): %d", id);

        return id;
    }
    
    // Try getLookID which works for most other brush types
    id = brush->getLookID();
    if (id > 0) {
        wxLogDebug("GetItemIDFromBrush: Got ID from getLookID(): %d", id);

        return id;
    }
    
    // Try specific brush type methods - when all else fails
    if (brush->isGround()) {
        wxLogDebug("GetItemIDFromBrush: Detected Ground brush");

        GroundBrush* groundBrush = brush->asGround();
        if (groundBrush) {
            // For ground brush, id is usually the server_lookid from grounds.xml
            // Try to find something else
            wxLogDebug("GetItemIDFromBrush: Failed to get ID for Ground brush");

        }
    }
    else if (brush->isWall()) {
        wxLogDebug("GetItemIDFromBrush: Detected Wall brush");

        WallBrush* wallBrush = brush->asWall();
        if (wallBrush) {
            wxLogDebug("GetItemIDFromBrush: Failed to get ID for Wall brush");

        }
    }
    else if (brush->isDoodad()) {
        wxLogDebug("GetItemIDFromBrush: Detected Doodad brush");

        DoodadBrush* doodadBrush = brush->asDoodad();
        if (doodadBrush) {
            wxLogDebug("GetItemIDFromBrush: Failed to get ID for Doodad brush");

        }
    }
    
    if (id == 0) {
        wxLogDebug("GetItemIDFromBrush: Failed to get item ID from brush %s", wxString(brush->getName()).c_str());

    }
    
    return id;
}

// Event table for BorderEditorDialog
BEGIN_EVENT_TABLE(BorderEditorDialog, wxDialog)
    EVT_BUTTON(wxID_ADD, BorderEditorDialog::OnAddItem)
    EVT_BUTTON(wxID_CLEAR, BorderEditorDialog::OnClear)
    EVT_BUTTON(wxID_SAVE, BorderEditorDialog::OnSave)
    EVT_BUTTON(wxID_CLOSE, BorderEditorDialog::OnClose)
    EVT_BUTTON(wxID_FIND, BorderEditorDialog::OnBrowse)
    EVT_COMBOBOX(wxID_ANY, BorderEditorDialog::OnLoadBorder)
    EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, BorderEditorDialog::OnPageChanged)
    EVT_BUTTON(wxID_ADD + 100, BorderEditorDialog::OnAddGroundItem)
    EVT_BUTTON(wxID_REMOVE, BorderEditorDialog::OnRemoveGroundItem)
    EVT_BUTTON(wxID_FIND + 100, BorderEditorDialog::OnGroundBrowse)
    EVT_COMBOBOX(wxID_ANY + 100, BorderEditorDialog::OnLoadGroundBrush)
END_EVENT_TABLE()

// Event table for BorderItemButton
BEGIN_EVENT_TABLE(BorderItemButton, wxButton)
    EVT_PAINT(BorderItemButton::OnPaint)
END_EVENT_TABLE()

// Event table for BorderGridPanel
BEGIN_EVENT_TABLE(BorderGridPanel, wxPanel)
    EVT_PAINT(BorderGridPanel::OnPaint)
    EVT_LEFT_UP(BorderGridPanel::OnMouseClick)
    EVT_LEFT_DOWN(BorderGridPanel::OnMouseDown)
END_EVENT_TABLE()

// Event table for BorderPreviewPanel
BEGIN_EVENT_TABLE(BorderPreviewPanel, wxPanel)
    EVT_PAINT(BorderPreviewPanel::OnPaint)
END_EVENT_TABLE()

BorderEditorDialog::BorderEditorDialog(wxWindow* parent, const wxString& title) :
    wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(850, 650),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_nextBorderId(1),
    m_activeTab(0) {
    
    CreateGUIControls();
    LoadExistingBorders();
    LoadExistingGroundBrushes();
    LoadExistingWallBrushes();
    
    // Default to border tab
    m_notebook->SetSelection(0);
    
    // Set ID to next available ID
    m_idCtrl->SetValue(m_nextBorderId);
    
    // Center the dialog
    Fit();
    CenterOnParent();
}

BorderEditorDialog::~BorderEditorDialog() {
    // Nothing to destroy manually
}

void BorderEditorDialog::CreateGUIControls() {
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    
    // Common properties - more compact horizontal layout
    // Common properties - cleaner layout
    wxBoxSizer* commonPropertiesSizer = new wxBoxSizer(wxVERTICAL);
    
    wxBoxSizer* commonPropertiesHorizSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Name field
    wxBoxSizer* nameSizer = new wxBoxSizer(wxVERTICAL);
    nameSizer->Add(new wxStaticText(this, wxID_ANY, "Name:"), 0);
    m_nameCtrl = new wxTextCtrl(this, wxID_ANY);
    m_nameCtrl->SetToolTip("Descriptive name for the border/brush");
    nameSizer->Add(m_nameCtrl, 0, wxEXPAND | wxTOP, 2);
    commonPropertiesHorizSizer->Add(nameSizer, 1, wxEXPAND | wxRIGHT, 10);
    
    // ID field
    wxBoxSizer* idSizer = new wxBoxSizer(wxVERTICAL);
    idSizer->Add(new wxStaticText(this, wxID_ANY, "ID:"), 0);
    m_idCtrl = new wxSpinCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1000);
    m_idCtrl->SetToolTip("Unique identifier for this border/brush");
    idSizer->Add(m_idCtrl, 0, wxEXPAND | wxTOP, 2);
    commonPropertiesHorizSizer->Add(idSizer, 0, wxEXPAND);
    
    commonPropertiesSizer->Add(commonPropertiesHorizSizer, 0, wxEXPAND | wxALL, 5);
    topSizer->Add(commonPropertiesSizer, 0, wxEXPAND | wxALL, 5);
    
    // Create notebook with Border and Ground tabs
    m_notebook = new wxNotebook(this, wxID_ANY);
    
    // ========== BORDER TAB ==========
    m_borderPanel = new wxPanel(m_notebook);
    wxBoxSizer* borderSizer = new wxBoxSizer(wxVERTICAL);
    
    // Border Properties - more compact layout
    // Border Properties - cleaner layout
    wxBoxSizer* borderPropertiesSizer = new wxBoxSizer(wxVERTICAL);
    
    // Two-column horizontal layout
    wxBoxSizer* borderPropsHorizSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Left column - Group and Type
    wxBoxSizer* leftColSizer = new wxBoxSizer(wxVERTICAL);
    
    // Border Group
    wxBoxSizer* groupSizer = new wxBoxSizer(wxVERTICAL);
    groupSizer->Add(new wxStaticText(m_borderPanel, wxID_ANY, "Group:"), 0);
    m_groupCtrl = new wxSpinCtrl(m_borderPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1000);
    m_groupCtrl->SetToolTip("Optional group identifier (0 = no group)");
    groupSizer->Add(m_groupCtrl, 0, wxEXPAND | wxTOP, 2);
    leftColSizer->Add(groupSizer, 0, wxEXPAND | wxBOTTOM, 5);
    
    // Border Type
    wxBoxSizer* typeSizer = new wxBoxSizer(wxVERTICAL);
    typeSizer->Add(new wxStaticText(m_borderPanel, wxID_ANY, "Type:"), 0);
    wxBoxSizer* checkboxSizer = new wxBoxSizer(wxHORIZONTAL);
    m_isOptionalCheck = new wxCheckBox(m_borderPanel, wxID_ANY, "Optional");
    m_isOptionalCheck->SetToolTip("Marks this border as optional");
    m_isGroundCheck = new wxCheckBox(m_borderPanel, wxID_ANY, "Ground");
    m_isGroundCheck->SetToolTip("Marks this border as a ground border");
    checkboxSizer->Add(m_isOptionalCheck, 0, wxRIGHT, 10);
    checkboxSizer->Add(m_isGroundCheck, 0);
    typeSizer->Add(checkboxSizer, 0, wxEXPAND | wxTOP, 2);
    leftColSizer->Add(typeSizer, 0, wxEXPAND);
    
    borderPropsHorizSizer->Add(leftColSizer, 1, wxEXPAND | wxRIGHT, 10);
    
    // Right column - Load Existing
    wxBoxSizer* rightColSizer = new wxBoxSizer(wxVERTICAL);
    rightColSizer->Add(new wxStaticText(m_borderPanel, wxID_ANY, "Load Existing:"), 0);
    m_existingBordersCombo = new wxComboBox(m_borderPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY | wxCB_DROPDOWN);
    m_existingBordersCombo->SetToolTip("Load an existing border as template");
    rightColSizer->Add(m_existingBordersCombo, 0, wxEXPAND | wxTOP, 2);
    
    borderPropsHorizSizer->Add(rightColSizer, 1, wxEXPAND);
    
    borderPropertiesSizer->Add(borderPropsHorizSizer, 0, wxEXPAND | wxALL, 5);
    borderSizer->Add(borderPropertiesSizer, 0, wxEXPAND | wxALL, 5);
    
    // Border content area with grid and preview
    wxBoxSizer* borderContentSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Left side - Grid Editor
    wxBoxSizer* gridSizer = new wxBoxSizer(wxVERTICAL);
    m_gridPanel = new BorderGridPanel(m_borderPanel);
    gridSizer->Add(m_gridPanel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    
    // Add instruction label
    wxStaticText* instructions = new wxStaticText(m_borderPanel, wxID_ANY, 
        "Click on a grid position to place the currently selected brush.\n"
        "The item ID will be extracted automatically from the brush.");
    instructions->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    gridSizer->Add(instructions, 0, wxEXPAND | wxALL, 5);
    
    // Current selected item controls
    wxBoxSizer* itemSizer = new wxBoxSizer(wxHORIZONTAL);
    itemSizer->Add(new wxStaticText(m_borderPanel, wxID_ANY, "Item ID:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_itemIdCtrl = new wxSpinCtrl(m_borderPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535);
    m_itemIdCtrl->SetToolTip("Enter an item ID manually if you don't want to use the current brush");
    itemSizer->Add(m_itemIdCtrl, 0, wxRIGHT, 5);
    wxButton* browseButton = new wxButton(m_borderPanel, wxID_FIND, "Browse...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    browseButton->SetToolTip("Browse for an item to use instead of the current brush");
    itemSizer->Add(browseButton, 0, wxRIGHT, 5);
    wxButton* addButton = new wxButton(m_borderPanel, wxID_ADD, "Add Manually", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    addButton->SetToolTip("Add the item ID manually to the currently selected position");
    itemSizer->Add(addButton, 0);
    
    gridSizer->Add(itemSizer, 0, wxEXPAND | wxALL, 5);
    
    // Add grid editor to content sizer
    borderContentSizer->Add(gridSizer, 1, wxEXPAND | wxALL, 5);
    
    // Right side - Preview Panel
    wxBoxSizer* previewSizer = new wxBoxSizer(wxVERTICAL);
    m_previewPanel = new BorderPreviewPanel(m_borderPanel);
    previewSizer->Add(m_previewPanel, 1, wxEXPAND | wxALL, 5);
    
    // Add preview to content sizer
    borderContentSizer->Add(previewSizer, 1, wxEXPAND | wxALL, 5);
    
    // Add content sizer to main border sizer
    borderSizer->Add(borderContentSizer, 1, wxEXPAND | wxALL, 5);
    
    // Bottom buttons for border tab
    wxBoxSizer* borderButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    borderButtonSizer->Add(new wxButton(m_borderPanel, wxID_CLEAR, "Clear"), 0, wxRIGHT, 5);
    borderButtonSizer->Add(new wxButton(m_borderPanel, wxID_SAVE, "Save Border"), 0, wxRIGHT, 5);
    borderButtonSizer->AddStretchSpacer(1);
    borderButtonSizer->Add(new wxButton(m_borderPanel, wxID_CLOSE, "Close"), 0);
    
    borderSizer->Add(borderButtonSizer, 0, wxEXPAND | wxALL, 5);
    
    m_borderPanel->SetSizer(borderSizer);
    
    // ========== GROUND TAB ==========
    m_groundPanel = new wxPanel(m_notebook);
    wxBoxSizer* groundSizer = new wxBoxSizer(wxVERTICAL);
    
    // Ground Brush Properties - cleaner layout
    wxBoxSizer* groundPropertiesSizer = new wxBoxSizer(wxVERTICAL);
    
    // Two rows of two columns each
    wxBoxSizer* topRowSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Tileset selector
    wxBoxSizer* tilesetSizer = new wxBoxSizer(wxVERTICAL);
    tilesetSizer->Add(new wxStaticText(m_groundPanel, wxID_ANY, "Tileset:"), 0);
    m_tilesetChoice = new wxChoice(m_groundPanel, wxID_ANY);
    m_tilesetChoice->SetToolTip("Select tileset to add this brush to");
    tilesetSizer->Add(m_tilesetChoice, 0, wxEXPAND | wxTOP, 2);
    topRowSizer->Add(tilesetSizer, 1, wxEXPAND | wxRIGHT, 10);
    
    // Server Look ID
    wxBoxSizer* serverIdSizer = new wxBoxSizer(wxVERTICAL);
    serverIdSizer->Add(new wxStaticText(m_groundPanel, wxID_ANY, "Server Look ID:"), 0);
    m_serverLookIdCtrl = new wxSpinCtrl(m_groundPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535);
    m_serverLookIdCtrl->SetToolTip("Server-side item ID");
    serverIdSizer->Add(m_serverLookIdCtrl, 0, wxEXPAND | wxTOP, 2);
    topRowSizer->Add(serverIdSizer, 1, wxEXPAND);
    
    groundPropertiesSizer->Add(topRowSizer, 0, wxEXPAND | wxALL, 5);
    
    // Second row
    wxBoxSizer* bottomRowSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Z-Order
    wxBoxSizer* zOrderSizer = new wxBoxSizer(wxVERTICAL);
    zOrderSizer->Add(new wxStaticText(m_groundPanel, wxID_ANY, "Z-Order:"), 0);
    m_zOrderCtrl = new wxSpinCtrl(m_groundPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10000);
    m_zOrderCtrl->SetToolTip("Z-Order for display");
    zOrderSizer->Add(m_zOrderCtrl, 0, wxEXPAND | wxTOP, 2);
    bottomRowSizer->Add(zOrderSizer, 1, wxEXPAND | wxRIGHT, 10);
    
    // Existing ground brushes dropdown
    wxBoxSizer* existingSizer = new wxBoxSizer(wxVERTICAL);
    existingSizer->Add(new wxStaticText(m_groundPanel, wxID_ANY, "Load Existing:"), 0);
    m_existingGroundBrushesCombo = new wxComboBox(m_groundPanel, wxID_ANY + 100, "", wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY | wxCB_DROPDOWN);
    m_existingGroundBrushesCombo->SetToolTip("Load an existing ground brush as template");
    existingSizer->Add(m_existingGroundBrushesCombo, 0, wxEXPAND | wxTOP, 2);
    bottomRowSizer->Add(existingSizer, 1, wxEXPAND);
    
    groundPropertiesSizer->Add(bottomRowSizer, 0, wxEXPAND | wxALL, 5);
    
    groundSizer->Add(groundPropertiesSizer, 0, wxEXPAND | wxALL, 5);
    
    // Ground Items
    wxBoxSizer* groundItemsSizer = new wxBoxSizer(wxVERTICAL);
    
    // List of ground items - set a smaller height
    m_groundItemsList = new wxListBox(m_groundPanel, ID_GROUND_ITEM_LIST, wxDefaultPosition, wxSize(-1, 100), 0, nullptr, wxLB_SINGLE);
    groundItemsSizer->Add(m_groundItemsList, 0, wxEXPAND | wxALL, 5);
    
    // Controls for adding/removing ground items
    wxBoxSizer* groundItemRowSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Left side - item ID and chance
    wxBoxSizer* itemDetailsSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Item ID input
    wxBoxSizer* itemIdSizer = new wxBoxSizer(wxVERTICAL);
    itemIdSizer->Add(new wxStaticText(m_groundPanel, wxID_ANY, "Item ID:"), 0);
    m_groundItemIdCtrl = new wxSpinCtrl(m_groundPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535);
    m_groundItemIdCtrl->SetToolTip("ID of the item to add");
    itemIdSizer->Add(m_groundItemIdCtrl, 0, wxEXPAND | wxTOP, 2);
    itemDetailsSizer->Add(itemIdSizer, 0, wxEXPAND | wxRIGHT, 5);
    
    // Chance input
    wxBoxSizer* chanceSizer = new wxBoxSizer(wxVERTICAL);
    chanceSizer->Add(new wxStaticText(m_groundPanel, wxID_ANY, "Chance:"), 0);
    m_groundItemChanceCtrl = new wxSpinCtrl(m_groundPanel, wxID_ANY, "10", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10000);
    m_groundItemChanceCtrl->SetToolTip("Chance of this item appearing");
    chanceSizer->Add(m_groundItemChanceCtrl, 0, wxEXPAND | wxTOP, 2);
    itemDetailsSizer->Add(chanceSizer, 0, wxEXPAND);
    
    groundItemRowSizer->Add(itemDetailsSizer, 1, wxEXPAND | wxRIGHT, 10);
    
    // Right side - buttons
    wxBoxSizer* itemButtonsSizer = new wxBoxSizer(wxVERTICAL);
    itemButtonsSizer->AddStretchSpacer();
    
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* groundBrowseButton = new wxButton(m_groundPanel, wxID_FIND + 100, "Browse...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    groundBrowseButton->SetToolTip("Browse for an item");
    buttonsSizer->Add(groundBrowseButton, 0, wxRIGHT, 5);
    
    wxButton* addGroundItemButton = new wxButton(m_groundPanel, wxID_ADD + 100, "Add", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    addGroundItemButton->SetToolTip("Add this item to the list");
    buttonsSizer->Add(addGroundItemButton, 0, wxRIGHT, 5);
    
    wxButton* removeGroundItemButton = new wxButton(m_groundPanel, wxID_REMOVE, "Remove", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    removeGroundItemButton->SetToolTip("Remove the selected item");
    buttonsSizer->Add(removeGroundItemButton, 0);
    
    itemButtonsSizer->Add(buttonsSizer, 0, wxEXPAND);
    groundItemRowSizer->Add(itemButtonsSizer, 0, wxEXPAND);
    
    groundItemsSizer->Add(groundItemRowSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    groundSizer->Add(groundItemsSizer, 0, wxEXPAND | wxALL, 5); // Changed from 1 to 0 to not expand
    
    // Grid and border selection for ground tab
    wxBoxSizer* groundBorderSizer = new wxBoxSizer(wxVERTICAL);
    
    // First row - Border alignment and 'to none' option
    wxBoxSizer* borderRow1 = new wxBoxSizer(wxHORIZONTAL);
    
    // Border alignment
    wxBoxSizer* alignSizer = new wxBoxSizer(wxVERTICAL);
    alignSizer->Add(new wxStaticText(m_groundPanel, wxID_ANY, "Border Alignment:"), 0);
    wxArrayString alignOptions;
    alignOptions.Add("outer");
    alignOptions.Add("inner");
    m_borderAlignmentChoice = new wxChoice(m_groundPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, alignOptions);
    m_borderAlignmentChoice->SetSelection(0); // Default to "outer"
    m_borderAlignmentChoice->SetToolTip("Alignment type for the border");
    alignSizer->Add(m_borderAlignmentChoice, 0, wxEXPAND | wxTOP, 2);
    borderRow1->Add(alignSizer, 1, wxEXPAND | wxRIGHT, 10);
    
    // Border options (checkboxes)
    wxBoxSizer* optionsSizer = new wxBoxSizer(wxVERTICAL);
    optionsSizer->Add(new wxStaticText(m_groundPanel, wxID_ANY, "Border Options:"), 0);
    wxBoxSizer* checksSizer = new wxBoxSizer(wxHORIZONTAL);
    m_includeToNoneCheck = new wxCheckBox(m_groundPanel, wxID_ANY, "To None");
    m_includeToNoneCheck->SetValue(true); // Default to checked
    m_includeToNoneCheck->SetToolTip("Adds additional border with 'to none' attribute");
    m_includeInnerCheck = new wxCheckBox(m_groundPanel, wxID_ANY, "Inner Border");
    m_includeInnerCheck->SetToolTip("Adds additional inner border with same ID");
    checksSizer->Add(m_includeToNoneCheck, 0, wxRIGHT, 10);
    checksSizer->Add(m_includeInnerCheck, 0);
    optionsSizer->Add(checksSizer, 0, wxEXPAND | wxTOP, 2);
    borderRow1->Add(optionsSizer, 1, wxEXPAND);
    
    groundBorderSizer->Add(borderRow1, 0, wxEXPAND | wxALL, 5);
    
    // Border ID notice (red text)
    wxBoxSizer* borderIdSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* borderIdLabel = new wxStaticText(m_groundPanel, wxID_ANY, "Border ID:");
    borderIdSizer->Add(borderIdLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    wxStaticText* borderId = new wxStaticText(m_groundPanel, wxID_ANY, "Uses the ID specified in 'Common Properties' section");
    borderId->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    borderIdSizer->Add(borderId, 1, wxALIGN_CENTER_VERTICAL);
    
    groundBorderSizer->Add(borderIdSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    
    // Grid use instruction - shorter text
    wxStaticText* gridInstructions = new wxStaticText(m_groundPanel, wxID_ANY, 
        "Use the grid in the Border tab to define borders for this ground brush.");
    gridInstructions->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    groundBorderSizer->Add(gridInstructions, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    
    groundSizer->Add(groundBorderSizer, 0, wxEXPAND | wxALL, 5);
    
    // Bottom buttons for ground tab
    wxBoxSizer* groundButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    groundButtonSizer->Add(new wxButton(m_groundPanel, wxID_CLEAR, "Clear"), 0, wxRIGHT, 5);
    groundButtonSizer->Add(new wxButton(m_groundPanel, wxID_SAVE, "Save Ground"), 0, wxRIGHT, 5);
    groundButtonSizer->AddStretchSpacer(1);
    groundButtonSizer->Add(new wxButton(m_groundPanel, wxID_CLOSE, "Close"), 0);
    
    groundSizer->Add(groundButtonSizer, 0, wxEXPAND | wxALL, 5);
    
    m_groundPanel->SetSizer(groundSizer);
    
    // Add tabs to notebook
    m_notebook->AddPage(m_borderPanel, "Border");
    m_notebook->AddPage(m_groundPanel, "Ground");
    
    // Notebook pages "Border Loop" and "Ground Brush" removed as they were duplicate pointers

    // ========== WALL TAB ==========
    m_wallPanel = new wxPanel(m_notebook);
    wxBoxSizer* wallSizer = new wxBoxSizer(wxVERTICAL);

    // Top: Existing Brushes + Common Wall Props
    wxBoxSizer* wallTopSizer = new wxBoxSizer(wxHORIZONTAL);

    // Left: Existing Brushes
    wxBoxSizer* existingWallSizer = new wxBoxSizer(wxVERTICAL);
    existingWallSizer->Add(new wxStaticText(m_wallPanel, wxID_ANY, "Load Existing:"), 0);
    m_existingWallBrushesCombo = new wxComboBox(m_wallPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY | wxCB_DROPDOWN);
    m_existingWallBrushesCombo->Bind(wxEVT_COMBOBOX, &BorderEditorDialog::OnLoadWallBrush, this);
    existingWallSizer->Add(m_existingWallBrushesCombo, 1, wxEXPAND | wxTOP, 2);
    wallTopSizer->Add(existingWallSizer, 1, wxEXPAND | wxRIGHT, 10);

    // Right: Server Look ID
    wxBoxSizer* wallServerIdSizer = new wxBoxSizer(wxVERTICAL);
    wallServerIdSizer->Add(new wxStaticText(m_wallPanel, wxID_ANY, "Server Look ID:"), 0);
    m_wallServerLookIdCtrl = new wxSpinCtrl(m_wallPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535);
    wallServerIdSizer->Add(m_wallServerLookIdCtrl, 0, wxEXPAND | wxTOP, 2);
    wallTopSizer->Add(wallServerIdSizer, 0, wxEXPAND);

    wallSizer->Add(wallTopSizer, 0, wxEXPAND | wxALL, 5);

    // Middle: Content Area (Structure Editor + Preview)
    wxBoxSizer* wallContentSizer = new wxBoxSizer(wxHORIZONTAL);

    // Left: Structure Editor
    wxStaticBoxSizer* structureSizer = new wxStaticBoxSizer(wxVERTICAL, m_wallPanel, "Wall Structure");
    
    // Type Selector
    wxBoxSizer* wallTypeSizer = new wxBoxSizer(wxHORIZONTAL);
    wallTypeSizer->Add(new wxStaticText(m_wallPanel, wxID_ANY, "Logic Type:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    
    wxArrayString wallTypes;
    wallTypes.Add("vertical");
    wallTypes.Add("horizontal");
    wallTypes.Add("corner");
    wallTypes.Add("pole");
    m_wallTypeChoice = new wxChoice(m_wallPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wallTypes);
    m_wallTypeChoice->SetSelection(0);
    m_wallTypeChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& ev) { UpdateWallItemsList(); });
    wallTypeSizer->Add(m_wallTypeChoice, 1, wxEXPAND);
    
    structureSizer->Add(wallTypeSizer, 0, wxEXPAND | wxALL, 5);

    // List of items for this type
    m_wallItemsList = new wxListBox(m_wallPanel, wxID_ANY, wxDefaultPosition, wxSize(-1, 100));
    structureSizer->Add(m_wallItemsList, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

    // Add Item Controls
    wxBoxSizer* wallItemInputSizer = new wxBoxSizer(wxHORIZONTAL);
    
    m_wallItemIdCtrl = new wxSpinCtrl(m_wallPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535);
    wallItemInputSizer->Add(m_wallItemIdCtrl, 0, wxRIGHT, 2);
    
    wxButton* wallBrowseBtn = new wxButton(m_wallPanel, wxID_ANY, "...", wxDefaultPosition, wxSize(30, -1));
    wallBrowseBtn->Bind(wxEVT_BUTTON, &BorderEditorDialog::OnWallBrowse, this);
    wallItemInputSizer->Add(wallBrowseBtn, 0, wxRIGHT, 5);

    wxButton* addWallItemBtn = new wxButton(m_wallPanel, wxID_ANY, "Add", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    addWallItemBtn->Bind(wxEVT_BUTTON, &BorderEditorDialog::OnAddWallItem, this);
    wallItemInputSizer->Add(addWallItemBtn, 0);

    structureSizer->Add(wallItemInputSizer, 0, wxEXPAND | wxALL, 5);
    
    wxButton* remWallItemBtn = new wxButton(m_wallPanel, wxID_ANY, "Remove Selected");
    remWallItemBtn->Bind(wxEVT_BUTTON, &BorderEditorDialog::OnRemoveWallItem, this);
    structureSizer->Add(remWallItemBtn, 0, wxALIGN_RIGHT | wxALL, 5);

    wallContentSizer->Add(structureSizer, 1, wxEXPAND | wxRIGHT, 5);

    // Right: Preview
    wxStaticBoxSizer* wallVisualSizer = new wxStaticBoxSizer(wxVERTICAL, m_wallPanel, "Preview");
    m_wallVisualPanel = new WallVisualPanel(m_wallPanel);
    wallVisualSizer->Add(m_wallVisualPanel, 1, wxEXPAND | wxALL, 5);
    wallContentSizer->Add(wallVisualSizer, 1, wxEXPAND);

    wallSizer->Add(wallContentSizer, 1, wxEXPAND | wxALL, 5);

    // Bottom buttons
    wxBoxSizer* wallButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    wallButtonSizer->Add(new wxButton(m_wallPanel, wxID_ANY, "Clear All"), 0, wxRIGHT, 5);
    wallButtonSizer->AddStretchSpacer(1);
    wxButton* saveWallButton = new wxButton(m_wallPanel, wxID_ANY, "Save Wall");
    saveWallButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& ev) { SaveWallBrush(); });
    wallButtonSizer->Add(saveWallButton, 0, wxRIGHT, 5);
    wallButtonSizer->Add(new wxButton(m_wallPanel, wxID_CLOSE, "Close"), 0);
    
    wallSizer->Add(wallButtonSizer, 0, wxEXPAND | wxALL, 5);
    
    m_wallPanel->SetSizer(wallSizer);
    m_notebook->AddPage(m_wallPanel, "Wall");
    
    topSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 5);
    
    SetSizer(topSizer);
    Layout();
}

void BorderEditorDialog::OnLoadWallBrush(wxCommandEvent& event) {
    int selection = m_existingWallBrushesCombo->GetSelection();
    if (selection == wxNOT_FOUND || selection == 0) { // 0 is <Create New>
        // Clear all fields for new brush
        if (m_wallServerLookIdCtrl) m_wallServerLookIdCtrl->SetValue(0);
        if (m_wallIsOptionalCheck) m_wallIsOptionalCheck->SetValue(false);
        if (m_wallIsGroundCheck) m_wallIsGroundCheck->SetValue(false);
        ClearWallItems();
        return;
    }
    
    // Get the name of the selected brush
    wxString name = m_existingWallBrushesCombo->GetString(selection);
    
    // Find the walls.xml file
    wxString dataDir = g_gui.GetDataDirectory();
    
    wxString wallsFile = dataDir + wxFileName::GetPathSeparator() + 
                        "materials" + wxFileName::GetPathSeparator() + "brushs" + wxFileName::GetPathSeparator() + "walls.xml";
    
    // Load XML
    pugi::xml_document doc;
    if (!doc.load_file(nstr(wallsFile).c_str())) return;
    
    pugi::xml_node materials = doc.child("materials");
    if (!materials) return;
    
    // Find the brush
    for (pugi::xml_node brushNode = materials.child("brush"); brushNode; brushNode = brushNode.next_sibling("brush")) {
        pugi::xml_attribute nameAttr = brushNode.attribute("name");
        if (nameAttr && wxString(nameAttr.as_string()) == name) {
            // Found it! Load properties
            
            // Server look ID
            pugi::xml_attribute lookIdAttr = brushNode.attribute("server_lookid");
            if (lookIdAttr && m_wallServerLookIdCtrl) {
                m_wallServerLookIdCtrl->SetValue(lookIdAttr.as_int());
            }
            
            // Clear existing items
            ClearWallItems();
            m_wallTypes.clear();
            
            // Iterate over children to find <wall> nodes
            for (pugi::xml_node wallNode = brushNode.child("wall"); wallNode; wallNode = wallNode.next_sibling("wall")) {
                pugi::xml_attribute typeAttr = wallNode.attribute("type");
                if (typeAttr) {
                    std::string typeName = typeAttr.as_string();
                    WallTypeData typeData;
                    typeData.typeName = typeName;
                    
                    // Parse items within this wall node
                    for (pugi::xml_node itemNode = wallNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item")) {
                        pugi::xml_attribute idAttr = itemNode.attribute("id");
                        pugi::xml_attribute chanceAttr = itemNode.attribute("chance");
                        
                        if (idAttr) {
                            uint16_t itemId = idAttr.as_uint();
                            int chance = chanceAttr ? chanceAttr.as_int() : 10;
                            typeData.items.push_back(GroundItem(itemId, chance));
                        }
                    }
                    
                    m_wallTypes[typeName] = typeData;
                }
            }
            
            // Update UI to reflect loaded data
            m_wallTypeChoice->SetSelection(0);
            UpdateWallItemsList();
            m_wallVisualPanel->SetWallItems(m_wallTypes);
            
            break;
        }
    }
}

void BorderEditorDialog::SaveWallBrush() {
    wxString name = m_existingWallBrushesCombo->GetValue();
    if (name.IsEmpty() || name == "<Create New>") {
        wxMessageBox("Please enter a name for the wall brush.", "Error", wxICON_ERROR);
        return;
    }
    
    int serverLookId = m_wallServerLookIdCtrl->GetValue();
    
    // Find walls.xml
    wxString dataDir = g_gui.GetDataDirectory();
    
    wxString wallsFile = dataDir + wxFileName::GetPathSeparator() + 
                        "materials" + wxFileName::GetPathSeparator() + "brushs" + wxFileName::GetPathSeparator() + "walls.xml";
                        
    pugi::xml_document doc;
    bool loaded = doc.load_file(nstr(wallsFile).c_str());
    if (!loaded) {
        // Create new if not exists?
        // Usually we expect it to exist.
        // If not, maybe create simple structure
        pugi::xml_node root = doc.append_child("materials");
    }
    
    pugi::xml_node materials = doc.child("materials");
    if (!materials) materials = doc.append_child("materials");
    
    // Check if exists and remove OLD entry to replace it completely
    pugi::xml_node existingNode;
    for (pugi::xml_node node = materials.child("brush"); node; node = node.next_sibling("brush")) {
        if (wxString(node.attribute("name").as_string()) == name) {
            existingNode = node;
            break;
        }
    }
    
    if (existingNode) {
        if (wxMessageBox("Wall brush '" + name + "' already exists. Overwrite?", "Confirm Overwrite", wxYES_NO | wxICON_QUESTION) != wxYES) {
            return;
        }
        materials.remove_child(existingNode);
    }
    
    // Create NEW brush node
    pugi::xml_node brushNode = materials.append_child("brush");
    brushNode.append_attribute("name").set_value(nstr(name).c_str());
    brushNode.append_attribute("type").set_value("wall");
    brushNode.append_attribute("server_lookid").set_value(serverLookId);
    
    // Iterate over our map and save <wall> nodes
    for (const auto& pair : m_wallTypes) {
        const WallTypeData& typeData = pair.second;
        if (typeData.items.empty()) continue; // Skip empty types
        
        pugi::xml_node wallNode = brushNode.append_child("wall");
        wallNode.append_attribute("type").set_value(typeData.typeName.c_str());
        
        for (const GroundItem& item : typeData.items) {
            pugi::xml_node itemNode = wallNode.append_child("item");
            itemNode.append_attribute("id").set_value(item.itemId);
            itemNode.append_attribute("chance").set_value(item.chance);
        }
    }
    
    // Save
    if (doc.save_file(nstr(wallsFile).c_str())) {
        wxMessageBox("Wall brush saved successfully.", "Success", wxICON_INFORMATION);
        LoadExistingWallBrushes(); // Reload list
        m_existingWallBrushesCombo->SetValue(name); // Restore selection
    } else {
        wxMessageBox("Failed to save walls.xml", "Error", wxICON_ERROR);
    }
}

void BorderEditorDialog::LoadExistingWallBrushes() {
    m_existingWallBrushesCombo->Clear();
    m_existingWallBrushesCombo->Append("<Create New>");
    m_existingWallBrushesCombo->SetSelection(0);
    
    wxString dataDir = g_gui.GetDataDirectory();
    
    wxString wallsFile = dataDir + wxFileName::GetPathSeparator() + 
                        "materials" + wxFileName::GetPathSeparator() + "brushs" + wxFileName::GetPathSeparator() + "walls.xml";
                        
    if (!wxFileExists(wallsFile)) return;
    
    pugi::xml_document doc;
    if (!doc.load_file(nstr(wallsFile).c_str())) return;
    
    pugi::xml_node materials = doc.child("materials");
    if (!materials) return;
    
    wxArrayString names;
    for (pugi::xml_node node = materials.child("brush"); node; node = node.next_sibling("brush")) {
        names.Add(wxString(node.attribute("name").as_string()));
    }
    names.Sort();
    m_existingWallBrushesCombo->Append(names);
}

bool BorderEditorDialog::ValidateWallBrush() {
    return true;
}

// Helper to get current selected type string
wxString GetCurrentWallType(wxChoice* choice) {
    if (!choice) return "vertical";
    int sel = choice->GetSelection();
    if (sel == wxNOT_FOUND) return "vertical";
    return choice->GetString(sel);
}

void BorderEditorDialog::ClearWallItems() {
    if (m_wallVisualPanel)
        m_wallVisualPanel->Clear();
    m_wallTypes.clear();
    if (m_wallItemsList)
        m_wallItemsList->Clear();
}

void BorderEditorDialog::UpdateWallItemsList() {
    if (!m_wallItemsList || !m_wallTypeChoice) return;
    
    m_wallItemsList->Clear();
    std::string type = GetCurrentWallType(m_wallTypeChoice).ToStdString();
    
    if (m_wallTypes.find(type) != m_wallTypes.end()) {
        const WallTypeData& data = m_wallTypes[type];
        for (const GroundItem& item : data.items) {
            m_wallItemsList->Append(wxString::Format("Item ID: %d", item.itemId));
        }
    }
}

void BorderEditorDialog::OnWallBrowse(wxCommandEvent& event) {
    // Reuse Ground Browse Logic or create new dialog
    // implementation pending... using manual ID for now
}

void BorderEditorDialog::OnAddWallItem(wxCommandEvent& event) {
    if (!m_wallItemIdCtrl) return;
    int id = m_wallItemIdCtrl->GetValue();
    if (id <= 0) return;
    
    std::string type = GetCurrentWallType(m_wallTypeChoice).ToStdString();
    
    WallTypeData& data = m_wallTypes[type];
    data.typeName = type;
    data.items.push_back(GroundItem(id, 10)); // Default chance 10
    
    UpdateWallItemsList();
    m_wallVisualPanel->SetWallItems(m_wallTypes);
}

void BorderEditorDialog::OnRemoveWallItem(wxCommandEvent& event) {
    if (!m_wallItemsList) return;
    int sel = m_wallItemsList->GetSelection();
    if (sel == wxNOT_FOUND) return;
    
    std::string type = GetCurrentWallType(m_wallTypeChoice).ToStdString();
    WallTypeData& data = m_wallTypes[type];
    
    if (sel < (int)data.items.size()) {
        data.items.erase(data.items.begin() + sel);
        UpdateWallItemsList();
        m_wallVisualPanel->SetWallItems(m_wallTypes);
    }
}

void BorderEditorDialog::LoadExistingBorders() {
    // Clear the combobox
    m_existingBordersCombo->Clear();
    
    // Add an empty entry
    m_existingBordersCombo->Append("<Create New>");
    m_existingBordersCombo->SetSelection(0);
    
    // Find the borders.xml file using the same version path conversion as in map_display.cpp
    wxString dataDir = g_gui.GetDataDirectory();
    
    // Get version string and convert to proper directory format
    // Construct borders.xml path
    wxString bordersFile = dataDir + wxFileName::GetPathSeparator() + 
                          "materials" + wxFileName::GetPathSeparator() + "borders.xml";
    
    if (!wxFileExists(bordersFile)) {
        wxMessageBox("Cannot find borders.xml file in the data directory.", "Error", wxICON_ERROR);
        return;
    }
    
    // Load the XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(nstr(bordersFile).c_str());
    
    if (!result) {
        wxMessageBox("Failed to load borders.xml: " + wxString(result.description()), "Error", wxICON_ERROR);
        return;
    }
    
    pugi::xml_node materials = doc.child("materials");
    if (!materials) {
        wxMessageBox("Invalid borders.xml file: missing 'materials' node", "Error", wxICON_ERROR);
        return;
    }
    
    int highestId = 0;
    
    // Parse all borders
    for (pugi::xml_node borderNode = materials.child("border"); borderNode; borderNode = borderNode.next_sibling("border")) {
        pugi::xml_attribute idAttr = borderNode.attribute("id");
        if (!idAttr) continue;
        
        int id = idAttr.as_int();
        if (id > highestId) {
            highestId = id;
        }
        
        // Get the comment node before this border for its description
        std::string description;
        pugi::xml_node commentNode = borderNode.previous_sibling();
        if (commentNode && commentNode.type() == pugi::node_comment) {
            description = commentNode.value();
            // Extract the actual comment text by removing XML comment markers
            description = description.c_str(); // Ensure we have a clean copy
            
            // Trim leading and trailing whitespace first
            description.erase(0, description.find_first_not_of(" \t\n\r"));
            description.erase(description.find_last_not_of(" \t\n\r") + 1);
            
            // Remove leading "<!--" if present
            if (description.substr(0, 4) == "<!--") {
                description.erase(0, 4);
                // Trim whitespace after removing the marker
                description.erase(0, description.find_first_not_of(" \t\n\r"));
            }
            
            // Remove trailing "-->" if present
            if (description.length() >= 3 && description.substr(description.length() - 3) == "-->") {
                description.erase(description.length() - 3);
                // Trim whitespace after removing the marker
                description.erase(description.find_last_not_of(" \t\n\r") + 1);
            }
        }
        
        // Add to combobox
        wxString label = wxString::Format("Border %d", id);
        if (!description.empty()) {
            label += wxString::Format(" (%s)", wxstr(description));
        }
        
        m_existingBordersCombo->Append(label, new wxStringClientData(wxString::Format("%d", id)));
    }
    
    // Set the next border ID to one higher than the highest found
    m_nextBorderId = highestId + 1;
    m_idCtrl->SetValue(m_nextBorderId);
}

void BorderEditorDialog::OnLoadBorder(wxCommandEvent& event) {
    int selection = m_existingBordersCombo->GetSelection();
    if (selection <= 0) {
        // Selected "Create New" or nothing
        ClearItems();
        return;
    }
    
    wxStringClientData* data = static_cast<wxStringClientData*>(m_existingBordersCombo->GetClientObject(selection));
    if (!data) return;
    
    int borderId = wxAtoi(data->GetData());
    
    // Find the borders.xml file using the same version path conversion as in LoadExistingBorders
    wxString dataDir = g_gui.GetDataDirectory();
    
    // Get version string and convert to proper directory format
    wxString bordersFile = dataDir + wxFileName::GetPathSeparator() + 
                          "materials" + wxFileName::GetPathSeparator() + "borders.xml";
    
    if (!wxFileExists(bordersFile)) {
        wxMessageBox("Cannot find borders.xml file in the data directory.", "Error", wxICON_ERROR);
        return;
    }
    
    // Load the XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(nstr(bordersFile).c_str());
    
    if (!result) {
        wxMessageBox("Failed to load borders.xml: " + wxString(result.description()), "Error", wxICON_ERROR);
        return;
    }
    
    // Clear existing items
    ClearItems();
    
    // Look for the border with the specified ID
    pugi::xml_node materials = doc.child("materials");
    for (pugi::xml_node borderNode = materials.child("border"); borderNode; borderNode = borderNode.next_sibling("border")) {
        pugi::xml_attribute idAttr = borderNode.attribute("id");
        if (!idAttr || idAttr.as_int() != borderId) continue;
        
        // Set the ID in the control
        m_idCtrl->SetValue(borderId);
        
        // Check for border type
        pugi::xml_attribute typeAttr = borderNode.attribute("type");
        if (typeAttr) {
            std::string type = typeAttr.as_string();
            m_isOptionalCheck->SetValue(type == "optional");
        } else {
            m_isOptionalCheck->SetValue(false);
        }
        
        // Check for border group
        pugi::xml_attribute groupAttr = borderNode.attribute("group");
        if (groupAttr) {
            m_groupCtrl->SetValue(groupAttr.as_int());
        } else {
            m_groupCtrl->SetValue(0);
        }
        
        // Get the comment node before this border for its description
        pugi::xml_node commentNode = borderNode.previous_sibling();
        if (commentNode && commentNode.type() == pugi::node_comment) {
            std::string description = commentNode.value();
            // Extract the actual comment text by removing XML comment markers
            description = description.c_str(); // Ensure we have a clean copy
            
            // Trim leading and trailing whitespace first
            description.erase(0, description.find_first_not_of(" \t\n\r"));
            description.erase(description.find_last_not_of(" \t\n\r") + 1);
            
            // Remove leading "<!--" if present
            if (description.substr(0, 4) == "<!--") {
                description.erase(0, 4);
                // Trim whitespace after removing the marker
                description.erase(0, description.find_first_not_of(" \t\n\r"));
            }
            
            // Remove trailing "-->" if present
            if (description.length() >= 3 && description.substr(description.length() - 3) == "-->") {
                description.erase(description.length() - 3);
                // Trim whitespace after removing the marker
                description.erase(description.find_last_not_of(" \t\n\r") + 1);
            }
            
            m_nameCtrl->SetValue(wxstr(description));
        } else {
            m_nameCtrl->SetValue("");
        }
        
        // Load all border items
        for (pugi::xml_node itemNode = borderNode.child("borderitem"); itemNode; itemNode = itemNode.next_sibling("borderitem")) {
            pugi::xml_attribute edgeAttr = itemNode.attribute("edge");
            pugi::xml_attribute itemAttr = itemNode.attribute("item");
            
            if (!edgeAttr || !itemAttr) continue;
            
            BorderEdgePosition pos = edgeStringToPosition(edgeAttr.as_string());
            uint16_t itemId = itemAttr.as_uint();
            
            if (pos != EDGE_NONE && itemId > 0) {
                m_borderItems.push_back(BorderItem(pos, itemId));
                m_gridPanel->SetItemId(pos, itemId);
            }
        }
        
        break;
    }
    
    // Update the preview
    UpdatePreview();
    
    // Keep selection
    m_existingBordersCombo->SetSelection(selection);
}

void BorderEditorDialog::OnItemIdChanged(wxCommandEvent& event) {
    // This event handler would update the display when an item ID is entered manually
    // but we're handling this directly in OnAddItem instead
}

void BorderEditorDialog::OnBrowse(wxCommandEvent& event) {
    // Open the Find Item dialog instead
    FindItemDialog dialog(this, "Select Border Item");
    
    if (dialog.ShowModal() == wxID_OK) {
        // Get the selected item ID
        uint16_t itemId = dialog.getResultID();
        
        // Find the item ID spin control
        wxSpinCtrl* itemIdCtrl = nullptr;
        wxWindowList& children = GetChildren();
        for (wxWindowList::iterator it = children.begin(); it != children.end(); ++it) {
            wxSpinCtrl* spinCtrl = dynamic_cast<wxSpinCtrl*>(*it);
            if (spinCtrl && spinCtrl != m_idCtrl) {
                itemIdCtrl = spinCtrl;
                break;
            }
        }
        
        if (itemIdCtrl && itemId > 0) {
            itemIdCtrl->SetValue(itemId);
        }
    }
}

void BorderEditorDialog::OnPositionSelected(wxCommandEvent& event) {
    // Get the position from the event
    BorderEdgePosition pos = static_cast<BorderEdgePosition>(event.GetInt());
    wxLogDebug("OnPositionSelected: Position %s selected", wxstr(edgePositionToString(pos)).c_str());

    
    // Get the item ID from the current brush
    Brush* currentBrush = g_gui.GetCurrentBrush();
    uint16_t itemId = 0;
    
    if (currentBrush) {
        wxLogDebug("OnPositionSelected: Using brush: %s", wxString(currentBrush->getName()).c_str());
        
        // Try to get the item ID directly - check if it's a RAW brush first
        if (currentBrush->isRaw()) {
            RAWBrush* rawBrush = currentBrush->asRaw();
            if (rawBrush) {
                itemId = rawBrush->getItemID();
                wxLogDebug("OnPositionSelected: Got item ID %d directly from RAW brush", itemId);
            }
        } 
        
        // If we didn't get an ID from the RAW brush method, try the generic method
        if (itemId == 0) {
            itemId = GetItemIDFromBrush(currentBrush);
            wxLogDebug("OnPositionSelected: Got item ID %d from GetItemIDFromBrush", itemId);
        }
    } else {
        wxLogDebug("OnPositionSelected: No current brush selected");
    }
    
    // If no valid ID from brush, check the manual input control
    if (itemId == 0 && m_itemIdCtrl) {
        itemId = m_itemIdCtrl->GetValue();
        if (itemId > 0) {
            wxLogDebug("OnPositionSelected: Using item ID %d from manual control", itemId);
        }
    }

    if (itemId > 0) {
        // Update the item ID control - keeps the UI in sync with our selection
        if (m_itemIdCtrl) {
            m_itemIdCtrl->SetValue(itemId);
        }
        
        // Add or update the border item
        bool updated = false;
        for (size_t i = 0; i < m_borderItems.size(); i++) {
            if (m_borderItems[i].position == pos) {
                m_borderItems[i].itemId = itemId;
                updated = true;
                wxLogDebug("OnPositionSelected: Updated existing border item at position %s", wxstr(edgePositionToString(pos)).c_str());

                break;
            }
        }
        
        if (!updated) {
            m_borderItems.push_back(BorderItem(pos, itemId));
            wxLogDebug("OnPositionSelected: Added new border item at position %s", wxstr(edgePositionToString(pos)).c_str());

        }
        
        // Update the grid panel
        m_gridPanel->SetItemId(pos, itemId);
        wxLogDebug("OnPositionSelected: Set grid panel item ID for position %s to %d", wxstr(edgePositionToString(pos)).c_str(), itemId);

        
        // Update the preview
        UpdatePreview();
        
        // Log the addition
        wxLogDebug("Added border item at position %s with item ID %d", 
                  wxstr(edgePositionToString(pos)).c_str(), itemId);

    } else {
        // If we couldn't get an item ID from the brush, check if there's a value in the item ID control
        itemId = m_itemIdCtrl->GetValue();
        
        if (itemId > 0) {
            // Use the value from the control to update/add the border item
            bool updated = false;
            for (size_t i = 0; i < m_borderItems.size(); i++) {
                if (m_borderItems[i].position == pos) {
                    m_borderItems[i].itemId = itemId;
                    updated = true;
                    break;
                }
            }
            
            if (!updated) {
                m_borderItems.push_back(BorderItem(pos, itemId));
            }
            
            // Update the grid panel
            m_gridPanel->SetItemId(pos, itemId);
            
            // Update the preview
            UpdatePreview();
            
            wxLogDebug("Used item ID %d from control for position %s", 
                       itemId, wxstr(edgePositionToString(pos)).c_str());

        } else {
            wxLogDebug("No valid item ID found from current brush: %s", wxString(currentBrush->getName()).c_str());

            wxMessageBox("Could not get a valid item ID from the current brush. Please select an item brush or use the Browse button to select an item manually.", "Invalid Brush", wxICON_INFORMATION);
        }
    }
}

void BorderEditorDialog::OnAddItem(wxCommandEvent& event) {
    // Get the currently selected position in the grid panel
    static BorderEdgePosition lastSelectedPos = EDGE_NONE;
    BorderEdgePosition selectedPos = m_gridPanel->GetSelectedPosition();
    
    // If no position is currently selected, use the last selected position
    if (selectedPos == EDGE_NONE) {
        selectedPos = lastSelectedPos;
    }
    
    if (selectedPos == EDGE_NONE) {
        wxMessageBox("Please select a position on the grid first by clicking on it.", "Error", wxICON_ERROR);
        return;
    }
    
    // Save this position for future use
    lastSelectedPos = selectedPos;
    
    // Get the item ID from the control (now using the class member)
    uint16_t itemId = m_itemIdCtrl->GetValue();
    
    if (itemId == 0) {
        wxMessageBox("Please enter a valid item ID or use the Browse button.", "Error", wxICON_ERROR);
        return;
    }
    
    // Add or update the border item
    bool updated = false;
    for (size_t i = 0; i < m_borderItems.size(); i++) {
        if (m_borderItems[i].position == selectedPos) {
            m_borderItems[i].itemId = itemId;
            updated = true;
            break;
        }
    }
    
    if (!updated) {
        m_borderItems.push_back(BorderItem(selectedPos, itemId));
    }
    
    // Update the grid panel
    m_gridPanel->SetItemId(selectedPos, itemId);
    
    // Update the preview
    UpdatePreview();
    
    // Log the addition for debugging
    wxLogDebug("Added item ID %d at position %s via Add button", 
               itemId, wxstr(edgePositionToString(selectedPos)).c_str());
}

void BorderEditorDialog::OnClear(wxCommandEvent& event) {
    if (m_activeTab == 0) {
        // Border tab
    ClearItems();
    } else {
        // Ground tab
        ClearGroundItems();
    }
}

void BorderEditorDialog::ClearItems() {
    m_borderItems.clear();
    m_gridPanel->Clear();
    m_previewPanel->Clear();
    
    // Reset controls to defaults
    m_idCtrl->SetValue(m_nextBorderId);
    m_nameCtrl->SetValue("");
    m_isOptionalCheck->SetValue(false);
    m_isGroundCheck->SetValue(false);
    m_groupCtrl->SetValue(0);
    
    // Set combo selection to "Create New"
    m_existingBordersCombo->SetSelection(0);
}

void BorderEditorDialog::UpdatePreview() {
    m_previewPanel->SetBorderItems(m_borderItems);
    m_previewPanel->Refresh();
}

bool BorderEditorDialog::ValidateBorder() {
    // Check for empty name
    if (m_nameCtrl->GetValue().IsEmpty()) {
        wxMessageBox("Please enter a name for the border.", "Validation Error", wxICON_ERROR);
        return false;
    }
    
    if (m_borderItems.empty()) {
        wxMessageBox("The border must have at least one item.", "Validation Error", wxICON_ERROR);
        return false;
    }
    
    // Check that there are no duplicate positions
    std::set<BorderEdgePosition> positions;
    for (const BorderItem& item : m_borderItems) {
        if (positions.find(item.position) != positions.end()) {
            wxMessageBox("The border contains duplicate positions.", "Validation Error", wxICON_ERROR);
            return false;
        }
        positions.insert(item.position);
    }
    
    // Check for ID validity
    int id = m_idCtrl->GetValue();
    if (id <= 0) {
        wxMessageBox("Border ID must be greater than 0.", "Validation Error", wxICON_ERROR);
        return false;
    }
    
    return true;
}

void BorderEditorDialog::SaveBorder() {
    if (!ValidateBorder()) {
        return;
    }
    
    // Get the border properties
    int id = m_idCtrl->GetValue();
    wxString name = m_nameCtrl->GetValue();
    
    // Double check that we have a name (it's also checked in ValidateBorder)
    if (name.IsEmpty()) {
        wxMessageBox("You must provide a name for the border.", "Error", wxICON_ERROR);
        return;
    }
    
    bool isOptional = m_isOptionalCheck->GetValue();
    bool isGround = m_isGroundCheck->GetValue();
    int group = m_groupCtrl->GetValue();
    
    // Find the borders.xml file using the same version path conversion
    wxString dataDir = g_gui.GetDataDirectory();
    
    // Get version string and convert to proper directory format
    wxString bordersFile = dataDir + wxFileName::GetPathSeparator() + 
                          "materials" + wxFileName::GetPathSeparator() + "borders.xml";
    
    if (!wxFileExists(bordersFile)) {
        wxMessageBox("Cannot find borders.xml file in the data directory.", "Error", wxICON_ERROR);
        return;
    }
    
    // Load the XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(nstr(bordersFile).c_str());
    
    if (!result) {
        wxMessageBox("Failed to load borders.xml: " + wxString(result.description()), "Error", wxICON_ERROR);
        return;
    }
    
    pugi::xml_node materials = doc.child("materials");
    if (!materials) {
        wxMessageBox("Invalid borders.xml file: missing 'materials' node", "Error", wxICON_ERROR);
        return;
    }
    
    // Check if a border with this ID already exists
    bool borderExists = false;
    pugi::xml_node existingBorder;
    
    for (pugi::xml_node borderNode = materials.child("border"); borderNode; borderNode = borderNode.next_sibling("border")) {
        pugi::xml_attribute idAttr = borderNode.attribute("id");
        if (idAttr && idAttr.as_int() == id) {
            borderExists = true;
            existingBorder = borderNode;
            break;
        }
    }
    
    if (borderExists) {
        // Check if there's a comment node before the existing border
        pugi::xml_node commentNode = existingBorder.previous_sibling();
        bool hadComment = (commentNode && commentNode.type() == pugi::node_comment);
        
        // Ask for confirmation to overwrite
        if (wxMessageBox("A border with ID " + wxString::Format("%d", id) + " already exists. Do you want to overwrite it?", 
                        "Confirm Overwrite", wxYES_NO | wxICON_QUESTION) != wxYES) {
            return;
        }
        
        // If there was a comment node, remove it too
        if (hadComment) {
            materials.remove_child(commentNode);
        }
        
        // Remove the existing border
        materials.remove_child(existingBorder);
    }
    
 
    
    // Create the new border node
    pugi::xml_node borderNode = materials.append_child("border");
    borderNode.append_attribute("id").set_value(id);
    
    if (isOptional) {
        borderNode.append_attribute("type").set_value("optional");
    }
    
    if (isGround) {
        borderNode.append_attribute("ground").set_value("true");
    }
    
    if (group > 0) {
        borderNode.append_attribute("group").set_value(group);
    }
    
    // Add all border items
    for (const BorderItem& item : m_borderItems) {
        pugi::xml_node itemNode = borderNode.append_child("borderitem");
        itemNode.append_attribute("edge").set_value(edgePositionToString(item.position).c_str());
        itemNode.append_attribute("item").set_value(item.itemId);
    }
    
    // Save the file
    if (!doc.save_file(nstr(bordersFile).c_str())) {
        wxMessageBox("Failed to save changes to borders.xml", "Error", wxICON_ERROR);
        return;
    }
    
    wxMessageBox("Border saved successfully.", "Success", wxICON_INFORMATION);
    
    // Reload the existing borders list
    LoadExistingBorders();
}

void BorderEditorDialog::OnSave(wxCommandEvent& event) {
    if (m_activeTab == 0) {
        // Border tab
        SaveBorder();
    } else if (m_activeTab == 1) {
        // Ground tab
        SaveGroundBrush();
    }
}

void BorderEditorDialog::OnClose(wxCommandEvent& event) {
    Close();
}

void BorderEditorDialog::OnGridCellClicked(wxMouseEvent& event) {
    // This event is handled by the BorderGridPanel directly
    event.Skip();
}

// ============================================================================
// BorderItemButton

BorderItemButton::BorderItemButton(wxWindow* parent, BorderEdgePosition position, wxWindowID id) :
    wxButton(parent, id, "", wxDefaultPosition, wxSize(32, 32)),
    m_itemId(0),
    m_position(position) {
    // Set up the button to show sprite graphics
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

BorderItemButton::~BorderItemButton() {
    // No need to destroy anything manually
}

void BorderItemButton::SetItemId(uint16_t id) {
    m_itemId = id;
    Refresh();
}

void BorderItemButton::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    
    // Draw the button background
    wxRect rect = GetClientRect();
    dc.SetBrush(wxBrush(GetBackgroundColour()));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(rect);
    
    // Draw the item sprite if available
    if (m_itemId > 0) {
        const ItemType& type = g_items.getItemType(m_itemId);
        if (type.id != 0) {
            Sprite* sprite = g_gui.gfx.getSprite(type.clientID);
            if (sprite) {
                sprite->DrawTo(&dc, SPRITE_SIZE_32x32, 0, 0, rect.GetWidth(), rect.GetHeight());
            }
        }
    }
    
    // Draw a border around the button if it's focused
    if (HasFocus()) {
        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(rect);
    }
}

// ============================================================================
// BorderGridPanel

BorderGridPanel::BorderGridPanel(wxWindow* parent, wxWindowID id) :
    wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN)
{
    m_items.clear();
    m_selectedPosition = EDGE_NONE;
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Enable Full Repaint on Resize for responsive drawing
    SetWindowStyle(GetWindowStyle() | wxFULL_REPAINT_ON_RESIZE);
}

BorderGridPanel::~BorderGridPanel() {
    // Nothing to destroy manually
}

void BorderGridPanel::SetItemId(BorderEdgePosition pos, uint16_t itemId) {
    if (pos >= 0 && pos < EDGE_COUNT) {
        m_items[pos] = itemId;
        Refresh();
    }
}

uint16_t BorderGridPanel::GetItemId(BorderEdgePosition pos) const {
    auto it = m_items.find(pos);
    if (it != m_items.end()) {
        return it->second;
    }
    return 0;
}

void BorderGridPanel::Clear() {
    for (auto& item : m_items) {
        item.second = 0;
    }
    Refresh();
}

void BorderGridPanel::SetSelectedPosition(BorderEdgePosition pos) {
    m_selectedPosition = pos;
    Refresh();
}

// Helper to calculate grid metrics
struct GridMetrics {
    int cellSize;
    int spacing;
    int startX;
    int startY;
    int normalX, normalY;
    int cornerX, cornerY;
    int diagX, diagY;
    
    GridMetrics(const wxSize& size) {
        int w = size.GetWidth();
        int h = size.GetHeight();
        int margin = 5;
        
        // We have 3 blocks of 2x2. 
        // Width = 6 * cell + 2 * spacing. 
        // Let spacing = cell / 2.
        // Total width units = 7.
        // Height = 2 * cell.
        
        cellSize = std::min((w - 2 * margin) / 7, (h - 2 * margin) / 2);
        if (cellSize < 16) cellSize = 16; // Minimum size
        
        spacing = cellSize / 2;
        int totalWidth = (2 * cellSize) * 3 + 2 * spacing;
        
        startX = (w - totalWidth) / 2;
        startY = (h - 2 * cellSize) / 2;
        
        normalX = startX;
        normalY = startY;
        
        cornerX = normalX + 2 * cellSize + spacing;
        cornerY = startY;
        
        diagX = cornerX + 2 * cellSize + spacing;
        diagY = startY;
    }
};

void BorderGridPanel::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    
    // Draw the panel background using system colors
    wxRect rect = GetClientRect();
    dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)));
    dc.Clear();
    
    GridMetrics m(rect.GetSize());
    
    // Draw the grid layout
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    
    // Helper function to draw a grid
    auto drawGrid = [&](int offsetX, int offsetY, int gridSize, int cellSize) {
        for (int i = 0; i <= gridSize; i++) {
            // Vertical lines
            dc.DrawLine(
                offsetX + i * cellSize, 
                offsetY, 
                offsetX + i * cellSize, 
                offsetY + gridSize * cellSize
            );
            
            // Horizontal lines
            dc.DrawLine(
                offsetX, 
                offsetY + i * cellSize, 
                offsetX + gridSize * cellSize, 
                offsetY + i * cellSize
            );
        }
    };
    
    // Draw the three grid sections
    drawGrid(m.normalX, m.normalY, 2, m.cellSize);
    drawGrid(m.cornerX, m.cornerY, 2, m.cellSize);
    drawGrid(m.diagX, m.diagY, 2, m.cellSize);
    
    // Set font for position labels
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    // Scale font size roughly
    font.SetPixelSize(wxSize(0, std::max(10, m.cellSize / 4)));
    dc.SetFont(font);
    
    int padding = 2; // Fixed small padding to maximize sprite size
    
    // Function to draw an item at a position
    auto drawItemAtPos = [&](BorderEdgePosition pos, int gridX, int gridY, int offsetX, int offsetY) {
        int x = offsetX + gridX * m.cellSize + padding;
        int y = offsetY + gridY * m.cellSize + padding;
        int size = m.cellSize - 2 * padding;
        
        // Highlight selected position
        if (pos == m_selectedPosition) {
            // "Selection Blue" border, transparent fill to keep sprite visible
            dc.SetPen(wxPen(wxColour(0, 120, 215), 2));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRectangle(x - padding, y - padding, m.cellSize, m.cellSize);
        }
        
        // Draw sprite if available
        uint16_t itemId = GetItemId(pos);
        if (itemId > 0) {
            const ItemType& type = g_items.getItemType(itemId);
            if (type.id != 0) {
                Sprite* sprite = g_gui.gfx.getSprite(type.clientID);
                if (sprite) {
                    // Local scaling: Draw to temp DC, extract bitmap, scale, then draw
                    wxMemoryDC tempDC;
                    wxBitmap tempBitmap(32, 32);
                    tempDC.SelectObject(tempBitmap);
                    tempDC.SetBackground(*wxTRANSPARENT_BRUSH);
                    tempDC.Clear();
                    
                    // Draw sprite at native size to temp DC
                    sprite->DrawTo(&tempDC, SPRITE_SIZE_32x32, 0, 0);
                    
                    tempDC.SelectObject(wxNullBitmap);
                    
                    // Convert to image, scale, and draw
                    wxImage img = tempBitmap.ConvertToImage();
                    if (img.IsOk()) {
                        img.Rescale(size, size, wxIMAGE_QUALITY_NEAREST);
                        wxBitmap scaledBmp(img);
                        dc.DrawBitmap(scaledBmp, x, y, true);
                    }
                }
            }
        } else {
            // Only draw text label if NO item is present to prevent overlap
            wxString label = wxstr(edgePositionToString(pos));
            wxSize textSize = dc.GetTextExtent(label);
            
            // Center text
            dc.DrawText(label, x + (size - textSize.GetWidth()) / 2, 
                      y + size - textSize.GetHeight());
        }
    };
    
    // Draw normal direction items
    drawItemAtPos(EDGE_N, 0, 0, m.normalX, m.normalY);
    drawItemAtPos(EDGE_E, 1, 0, m.normalX, m.normalY);
    drawItemAtPos(EDGE_S, 0, 1, m.normalX, m.normalY);
    drawItemAtPos(EDGE_W, 1, 1, m.normalX, m.normalY);
    
    // Draw corner items
    drawItemAtPos(EDGE_CNW, 0, 0, m.cornerX, m.cornerY);
    drawItemAtPos(EDGE_CNE, 1, 0, m.cornerX, m.cornerY);
    drawItemAtPos(EDGE_CSW, 0, 1, m.cornerX, m.cornerY);
    drawItemAtPos(EDGE_CSE, 1, 1, m.cornerX, m.cornerY);
    
    // Draw diagonal items
    drawItemAtPos(EDGE_DNW, 0, 0, m.diagX, m.diagY);
    drawItemAtPos(EDGE_DNE, 1, 0, m.diagX, m.diagY);
    drawItemAtPos(EDGE_DSW, 0, 1, m.diagX, m.diagY);
    drawItemAtPos(EDGE_DSE, 1, 1, m.diagX, m.diagY);
}

wxPoint BorderGridPanel::GetPositionCoordinates(BorderEdgePosition pos) const {
    switch (pos) {
        case EDGE_N:   return wxPoint(1, 0);
        case EDGE_E:   return wxPoint(2, 1);
        case EDGE_S:   return wxPoint(1, 2);
        case EDGE_W:   return wxPoint(0, 1);
        case EDGE_CNW: return wxPoint(0, 0);
        case EDGE_CNE: return wxPoint(2, 0);
        case EDGE_CSE: return wxPoint(2, 2);
        case EDGE_CSW: return wxPoint(0, 2);
        case EDGE_DNW: return wxPoint(0.5, 0.5);
        case EDGE_DNE: return wxPoint(1.5, 0.5);
        case EDGE_DSE: return wxPoint(1.5, 1.5);
        case EDGE_DSW: return wxPoint(0.5, 1.5);
        default:       return wxPoint(-1, -1);
    }
}

BorderEdgePosition BorderGridPanel::GetPositionFromCoordinates(int x, int y) const
{
    GridMetrics m(GetClientSize());
    
    const int block_width = 2 * m.cellSize;
    const int block_height = 2 * m.cellSize;
    
    // Check which grid section the click is in and calculate the grid cell
    
    // Normal grid
    if (x >= m.normalX && x < m.normalX + block_width &&
        y >= m.normalY && y < m.normalY + block_height) {
        
        int gridX = (x - m.normalX) / m.cellSize;
        int gridY = (y - m.normalY) / m.cellSize;
        
        if (gridX == 0 && gridY == 0) return EDGE_N;
        if (gridX == 1 && gridY == 0) return EDGE_E;
        if (gridX == 0 && gridY == 1) return EDGE_S;
        if (gridX == 1 && gridY == 1) return EDGE_W;
    }
    
    // Corner grid
    if (x >= m.cornerX && x < m.cornerX + block_width &&
        y >= m.cornerY && y < m.cornerY + block_height) {
        
        int gridX = (x - m.cornerX) / m.cellSize;
        int gridY = (y - m.cornerY) / m.cellSize;
        
        if (gridX == 0 && gridY == 0) return EDGE_CNW;
        if (gridX == 1 && gridY == 0) return EDGE_CNE;
        if (gridX == 0 && gridY == 1) return EDGE_CSW;
        if (gridX == 1 && gridY == 1) return EDGE_CSE;
    }
    
    // Diagonal grid
    if (x >= m.diagX && x < m.diagX + block_width &&
        y >= m.diagY && y < m.diagY + block_height) {
        
        int gridX = (x - m.diagX) / m.cellSize;
        int gridY = (y - m.diagY) / m.cellSize;
        
        if (gridX == 0 && gridY == 0) return EDGE_DNW;
        if (gridX == 1 && gridY == 0) return EDGE_DNE;
        if (gridX == 0 && gridY == 1) return EDGE_DSW;
        if (gridX == 1 && gridY == 1) return EDGE_DSE;
    }
    
    return EDGE_NONE;
}

wxSize BorderGridPanel::DoGetBestSize() const {
    const int grid_cell_size = 64;
    const int SECTION_SPACING = 20;
    const int MARGIN = 10;
    
    // 3 blocks of 2 cells each, plus 2 spacings, plus side margins
    int width = MARGIN + (2 * grid_cell_size) + SECTION_SPACING + 
                (2 * grid_cell_size) + SECTION_SPACING + 
                (2 * grid_cell_size) + MARGIN;
                
    // 1 block height plus margins
    int height = MARGIN + (2 * grid_cell_size) + MARGIN;
    
    return wxSize(width, height);
}

void BorderGridPanel::OnMouseClick(wxMouseEvent& event) {
    int x = event.GetX();
    int y = event.GetY();
    
    BorderEdgePosition pos = GetPositionFromCoordinates(x, y);
    if (pos != EDGE_NONE) {
        // Set the position as selected in the grid
        SetSelectedPosition(pos);
        
        // Notify the parent dialog that a position was selected
        wxCommandEvent selEvent(wxEVT_COMMAND_BUTTON_CLICKED, ID_BORDER_GRID_SELECT);
        selEvent.SetInt(static_cast<int>(pos));
        
        // Find the parent BorderEditorDialog
        wxWindow* parent = GetParent();
        while (parent && !dynamic_cast<BorderEditorDialog*>(parent)) {
            parent = parent->GetParent();
        }
        
        // Send the event to the parent dialog
        BorderEditorDialog* dialog = dynamic_cast<BorderEditorDialog*>(parent);
        if (dialog) {
            // Call the event handler directly
            wxLogDebug("BorderGridPanel::OnMouseClick: Calling OnPositionSelected directly for position %s", wxstr(edgePositionToString(pos)));
            dialog->OnPositionSelected(selEvent);
        } else {
            // If we couldn't find the parent dialog, post the event to the parent
            wxLogDebug("BorderGridPanel::OnMouseClick: Could not find BorderEditorDialog parent, posting event");
            wxPostEvent(GetParent(), selEvent);
        }
    }
}

void BorderGridPanel::OnMouseDown(wxMouseEvent& event) {
    // Get the position from the coordinates
    BorderEdgePosition pos = GetPositionFromCoordinates(event.GetX(), event.GetY());
    
    // Set the selected position
    SetSelectedPosition(pos);
    
    event.Skip();
}

// ============================================================================
// BorderPreviewPanel

BorderPreviewPanel::BorderPreviewPanel(wxWindow* parent, wxWindowID id) :
    wxPanel(parent, id, wxDefaultPosition, wxDefaultSize) { // Use default size, let sizer expand
    // Set up the panel to handle painting
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Enable Full Repaint on Resize
    SetWindowStyle(GetWindowStyle() | wxFULL_REPAINT_ON_RESIZE);
}

BorderPreviewPanel::~BorderPreviewPanel() {
    // Nothing to destroy manually
}

void BorderPreviewPanel::SetBorderItems(const std::vector<BorderItem>& items) {
    m_borderItems = items;
    Refresh();
}

void BorderPreviewPanel::Clear() {
    m_borderItems.clear();
    Refresh();
}

void BorderPreviewPanel::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    
    // Draw the panel background
    wxRect rect = GetClientRect();
    dc.SetBackground(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE)));
    dc.Clear();
    
    // Fixed: 5x5 Cross Test Pattern for comprehensive border verification
    const int GRID_SIZE = 5;
    const int preview_cell_size = 32;
    const int total_grid_size = GRID_SIZE * preview_cell_size;

    // Center grid in the panel
    int startX = (rect.GetWidth() - total_grid_size) / 2;
    int startY = (rect.GetHeight() - total_grid_size) / 2;
    
    // Visual Polish: Add "Preview" label at the top
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    font.MakeBold();
    dc.SetFont(font);
    
    wxString label = "Preview (5x5 Test Pattern)";
    wxSize textSize = dc.GetTextExtent(label);
    dc.DrawText(label, (rect.GetWidth() - textSize.GetWidth()) / 2, 10);
    
    // Ensure grid doesn't overlap title if window is crunched
    if (startY < textSize.GetHeight() + 20) {
        startY = textSize.GetHeight() + 20;
    }
    
    // Draw grid lines for the 5x5 area
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT)));
    
    // Vertical lines
    for (int i = 0; i <= GRID_SIZE; i++) {
        dc.DrawLine(startX + i * preview_cell_size, startY, 
                    startX + i * preview_cell_size, startY + total_grid_size);
    }
    
    // Horizontal lines
    for (int i = 0; i <= GRID_SIZE; i++) {
        dc.DrawLine(startX, startY + i * preview_cell_size, 
                    startX + total_grid_size, startY + i * preview_cell_size);
    }
    
    // Helper lambda: Get the target edge for a grid position (x, y)
    auto getEdgeAtPosition = [](int x, int y) -> BorderEdgePosition {
        // Row 0
        if (y == 0) {
            if (x == 1) return EDGE_CSE;
            if (x == 2) return EDGE_S;
            if (x == 3) return EDGE_CSW;
        }
        // Row 1
        else if (y == 1) {
            if (x == 0) return EDGE_CSE;
            if (x == 1) return EDGE_DSE;
            if (x == 3) return EDGE_DSW;
            if (x == 4) return EDGE_CSW;
        }
        // Row 2 (Center row)
        else if (y == 2) {
            if (x == 0) return EDGE_E;
            if (x == 4) return EDGE_W;
            // x=2 is center ground (empty)
        }
        // Row 3
        else if (y == 3) {
            if (x == 0) return EDGE_CNE;
            if (x == 1) return EDGE_DNE;
            if (x == 3) return EDGE_DNW;
            if (x == 4) return EDGE_CNW;
        }
        // Row 4
        else if (y == 4) {
            if (x == 1) return EDGE_CNE;
            if (x == 2) return EDGE_N;
            if (x == 3) return EDGE_CNW;
        }
        
        return static_cast<BorderEdgePosition>(-1); // No edge for this cell
    };
    
    // Draw border items based on the cross pattern
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            BorderEdgePosition targetEdge = getEdgeAtPosition(x, y);
            
            if (targetEdge == static_cast<BorderEdgePosition>(-1)) {
                continue; // Empty cell
            }
            
            // Search for an item with this edge position
            for (const BorderItem& item : m_borderItems) {
                if (item.position == targetEdge) {
                    int cellX = startX + x * preview_cell_size;
                    int cellY = startY + y * preview_cell_size;
                    
                    // Draw the item sprite
                    const ItemType& type = g_items.getItemType(item.itemId);
                    if (type.id != 0) {
                        Sprite* sprite = g_gui.gfx.getSprite(type.clientID);
                        if (sprite) {
                            // Draw exactly at 32x32
                            sprite->DrawTo(&dc, SPRITE_SIZE_32x32, cellX, cellY, preview_cell_size, preview_cell_size);
                        }
                    }
                    break; // Found and drawn, move to next grid cell
                }
            }
        }
    }
}

void BorderEditorDialog::LoadExistingGroundBrushes() {
    // Clear the combo box
    m_existingGroundBrushesCombo->Clear();
    
    // Add "Create New" as the first option
    m_existingGroundBrushesCombo->Append("<Create New>");
    m_existingGroundBrushesCombo->SetSelection(0);
    
    // Find the grounds.xml file based on the current version
    wxString dataDir = g_gui.GetDataDirectory();
    
    // Get version string and convert to proper directory format
    wxString groundsFile = dataDir + wxFileName::GetPathSeparator() + 
                          "materials" + wxFileName::GetPathSeparator() + "brushs" + wxFileName::GetPathSeparator() + "grounds.xml";
    
    if (!wxFileExists(groundsFile)) {
        wxMessageBox("Cannot find grounds.xml file in the data directory.", "Error", wxICON_ERROR);
        return;
    }
    
    // Load the XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(nstr(groundsFile).c_str());
    
    if (!result) {
        wxMessageBox("Failed to load grounds.xml: " + wxString(result.description()), "Error", wxICON_ERROR);
        return;
    }
    
    // Look for all brush nodes
    pugi::xml_node materials = doc.child("materials");
    if (!materials) {
        wxMessageBox("Invalid grounds.xml file: missing 'materials' node", "Error", wxICON_ERROR);
        return;
    }
    
    for (pugi::xml_node brushNode = materials.child("brush"); brushNode; brushNode = brushNode.next_sibling("brush")) {
        pugi::xml_attribute nameAttr = brushNode.attribute("name");
        pugi::xml_attribute serverLookIdAttr = brushNode.attribute("server_lookid");
        pugi::xml_attribute typeAttr = brushNode.attribute("type");
        
        // Only include ground brushes
        if (!typeAttr || std::string(typeAttr.as_string()) != "ground") {
            continue;
        }
        
        if (nameAttr && serverLookIdAttr) {
            wxString brushName = wxString(nameAttr.as_string());
            int serverId = serverLookIdAttr.as_int();
            
            // Add the brush name to the combo box with the serverId as client data
            wxStringClientData* data = new wxStringClientData(wxString::Format("%d", serverId));
            m_existingGroundBrushesCombo->Append(brushName, data);
        }
    }
}

void BorderEditorDialog::ClearGroundItems() {
    m_groundItems.clear();
    m_nameCtrl->SetValue("");
    m_idCtrl->SetValue(m_nextBorderId);
    m_serverLookIdCtrl->SetValue(0);
    m_zOrderCtrl->SetValue(0);
    m_groundItemIdCtrl->SetValue(0);
    m_groundItemChanceCtrl->SetValue(10);
    
    // Reset border alignment options
    m_borderAlignmentChoice->SetSelection(0); // Default to "outer"
    m_includeToNoneCheck->SetValue(true);     // Default to checked
    m_includeInnerCheck->SetValue(false);     // Default to unchecked
    
    UpdateGroundItemsList();
}

void BorderEditorDialog::UpdateGroundItemsList() {
    m_groundItemsList->Clear();
    
    for (const GroundItem& item : m_groundItems) {
        wxString itemText = wxString::Format("Item ID: %d, Chance: %d", item.itemId, item.chance);
        m_groundItemsList->Append(itemText);
    }
}

void BorderEditorDialog::OnPageChanged(wxBookCtrlEvent& event) {
    m_activeTab = event.GetSelection();
    
    // When switching to the ground tab, use the same border items for the ground brush
    if (m_activeTab == 1) {
        // Update the ground items preview (not implemented yet)
    }
}

void BorderEditorDialog::OnAddGroundItem(wxCommandEvent& event) {
    uint16_t itemId = m_groundItemIdCtrl->GetValue();
    int chance = m_groundItemChanceCtrl->GetValue();
    
    if (itemId > 0) {
        // Check if this item already exists, if so update its chance
        bool updated = false;
        for (size_t i = 0; i < m_groundItems.size(); i++) {
            if (m_groundItems[i].itemId == itemId) {
                m_groundItems[i].chance = chance;
                updated = true;
                break;
            }
        }
        
        if (!updated) {
            // Add new item
            m_groundItems.push_back(GroundItem(itemId, chance));
        }
        
        // Update the list
        UpdateGroundItemsList();
    }
}

void BorderEditorDialog::OnRemoveGroundItem(wxCommandEvent& event) {
    int selection = m_groundItemsList->GetSelection();
    if (selection != wxNOT_FOUND && selection < static_cast<int>(m_groundItems.size())) {
        m_groundItems.erase(m_groundItems.begin() + selection);
        UpdateGroundItemsList();
    }
}

void BorderEditorDialog::OnGroundBrowse(wxCommandEvent& event) {
    // Open the Find Item dialog to select a ground item
    FindItemDialog dialog(this, "Select Ground Item");
    
    if (dialog.ShowModal() == wxID_OK) {
        // Get the selected item ID
        uint16_t itemId = dialog.getResultID();
        
        // Set the ground item ID field
        if (itemId > 0) {
            m_groundItemIdCtrl->SetValue(itemId);
        }
    }
}

void BorderEditorDialog::OnLoadGroundBrush(wxCommandEvent& event) {
    int selection = m_existingGroundBrushesCombo->GetSelection();
    if (selection <= 0) {
        // Selected "Create New" or nothing
        ClearGroundItems();
        return;
    }
    
    wxStringClientData* data = static_cast<wxStringClientData*>(m_existingGroundBrushesCombo->GetClientObject(selection));
    if (!data) return;
    
    int serverLookId = wxAtoi(data->GetData());
    
    // Find the grounds.xml file using the same version path conversion
    wxString dataDir = g_gui.GetDataDirectory();
    
    // Get version string and convert to proper directory format
    wxString groundsFile = dataDir + wxFileName::GetPathSeparator() + 
                          "materials" + wxFileName::GetPathSeparator() + "brushs" + wxFileName::GetPathSeparator() + "grounds.xml";
    
    if (!wxFileExists(groundsFile)) {
        wxMessageBox("Cannot find grounds.xml file in the data directory.", "Error", wxICON_ERROR);
        return;
    }
    
    // Load the XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(nstr(groundsFile).c_str());
    
    if (!result) {
        wxMessageBox("Failed to load grounds.xml: " + wxString(result.description()), "Error", wxICON_ERROR);
        return;
    }
    
    // Clear existing items
    ClearGroundItems();
    
    // Find the brush with the specified server_lookid
    pugi::xml_node materials = doc.child("materials");
    if (!materials) {
        wxMessageBox("Invalid grounds.xml file: missing 'materials' node", "Error", wxICON_ERROR);
        return;
    }
    
    for (pugi::xml_node brushNode = materials.child("brush"); brushNode; brushNode = brushNode.next_sibling("brush")) {
        pugi::xml_attribute serverLookIdAttr = brushNode.attribute("server_lookid");
        
        if (serverLookIdAttr && serverLookIdAttr.as_int() == serverLookId) {
            // Found the brush, load its properties
            pugi::xml_attribute nameAttr = brushNode.attribute("name");
            pugi::xml_attribute zOrderAttr = brushNode.attribute("z-order");
            
            if (nameAttr) {
                m_nameCtrl->SetValue(wxString(nameAttr.as_string()));
            }
            
            m_serverLookIdCtrl->SetValue(serverLookId);
            
            if (zOrderAttr) {
                m_zOrderCtrl->SetValue(zOrderAttr.as_int());
            }
            
            // Load all item nodes
            for (pugi::xml_node itemNode = brushNode.child("item"); itemNode; itemNode = itemNode.next_sibling("item")) {
                pugi::xml_attribute idAttr = itemNode.attribute("id");
                pugi::xml_attribute chanceAttr = itemNode.attribute("chance");
                
                if (idAttr) {
                    uint16_t itemId = idAttr.as_uint();
                    int chance = chanceAttr ? chanceAttr.as_int() : 10;
                    
                    m_groundItems.push_back(GroundItem(itemId, chance));
                }
            }
            
            // Load all border nodes and add to the border grid
            m_borderItems.clear();
            m_gridPanel->Clear();
            
            // Reset alignment options
            m_borderAlignmentChoice->SetSelection(0); // Default to "outer"
            m_includeToNoneCheck->SetValue(true);     // Default to checked
            m_includeInnerCheck->SetValue(false);     // Default to unchecked
            
            bool hasNormalBorder = false;
            bool hasToNoneBorder = false;
            bool hasInnerBorder = false;
            bool hasInnerToNoneBorder = false;
            wxString alignment = "outer"; // Default
            
            for (pugi::xml_node borderNode = brushNode.child("border"); borderNode; borderNode = borderNode.next_sibling("border")) {
                pugi::xml_attribute alignAttr = borderNode.attribute("align");
                pugi::xml_attribute toAttr = borderNode.attribute("to");
                pugi::xml_attribute idAttr = borderNode.attribute("id");
                
                if (idAttr) {
                    int borderId = idAttr.as_int();
                    m_idCtrl->SetValue(borderId);
                    
                    // Check border type and attributes
                    if (alignAttr) {
                        wxString alignVal = wxString(alignAttr.as_string());
                        
                        if (alignVal == "outer") {
                            if (toAttr && wxString(toAttr.as_string()) == "none") {
                                hasToNoneBorder = true;
                            } else {
                                hasNormalBorder = true;
                                alignment = "outer";
                            }
                        } else if (alignVal == "inner") {
                            if (toAttr && wxString(toAttr.as_string()) == "none") {
                                hasInnerToNoneBorder = true;
                            } else {
                                hasInnerBorder = true;
                            }
                        }
                    }
                    
                    // Load the border details from borders.xml
                    wxString bordersFile = dataDir + wxFileName::GetPathSeparator() + 
                                         "materials" + wxFileName::GetPathSeparator() + "borders.xml";
                    
                    if (!wxFileExists(bordersFile)) {
                        // Just skip if we can't find borders.xml
                        continue;
                    }
                    
                    pugi::xml_document bordersDoc;
                    pugi::xml_parse_result bordersResult = bordersDoc.load_file(nstr(bordersFile).c_str());
                    
                    if (!bordersResult) {
                        // Skip if we can't load borders.xml
                        continue;
                    }
                    
                    pugi::xml_node bordersMaterials = bordersDoc.child("materials");
                    if (!bordersMaterials) {
                        continue;
                    }
                    
                    for (pugi::xml_node targetBorder = bordersMaterials.child("border"); targetBorder; targetBorder = targetBorder.next_sibling("border")) {
                        pugi::xml_attribute targetIdAttr = targetBorder.attribute("id");
                        
                        if (targetIdAttr && targetIdAttr.as_int() == borderId) {
                            // Found the border, load its items
                            for (pugi::xml_node borderItemNode = targetBorder.child("borderitem"); borderItemNode; borderItemNode = borderItemNode.next_sibling("borderitem")) {
                                pugi::xml_attribute edgeAttr = borderItemNode.attribute("edge");
                                pugi::xml_attribute itemAttr = borderItemNode.attribute("item");
                                
                                if (!edgeAttr || !itemAttr) continue;
                                
                                BorderEdgePosition pos = edgeStringToPosition(edgeAttr.as_string());
                                uint16_t borderItemId = itemAttr.as_uint();
                                
                                if (pos != EDGE_NONE && borderItemId > 0) {
                                    m_borderItems.push_back(BorderItem(pos, borderItemId));
                                    m_gridPanel->SetItemId(pos, borderItemId);
                                }
                            }
                            
                            break;
                        }
                    }
                }
            }
            
            // Update the ground items list and border preview
            UpdateGroundItemsList();
            UpdatePreview();
            
            // Apply the loaded border alignment settings
            if (hasInnerBorder) {
                m_includeInnerCheck->SetValue(true);
            }
            
            if (!hasToNoneBorder) {
                m_includeToNoneCheck->SetValue(false);
            }
            
            int alignmentIndex = 0; // Default to outer
            if (alignment == "inner") {
                alignmentIndex = 1;
            }
            m_borderAlignmentChoice->SetSelection(alignmentIndex);
            
            break;
        }
    }
    
    // Keep selection
    m_existingGroundBrushesCombo->SetSelection(selection);
}

bool BorderEditorDialog::ValidateGroundBrush() {
    // Check for empty name
    if (m_nameCtrl->GetValue().IsEmpty()) {
        wxMessageBox("Please enter a name for the ground brush.", "Validation Error", wxICON_ERROR);
        return false;
    }
    
    if (m_groundItems.empty()) {
        wxMessageBox("The ground brush must have at least one item.", "Validation Error", wxICON_ERROR);
        return false;
    }
    
    if (m_serverLookIdCtrl->GetValue() <= 0) {
        wxMessageBox("You must specify a valid server look ID.", "Validation Error", wxICON_ERROR);
        return false;
    }
    
    // Check tileset selection
    if (m_tilesetChoice->GetSelection() == wxNOT_FOUND) {
        wxMessageBox("Please select a tileset for the ground brush.", "Validation Error", wxICON_ERROR);
        return false;
    }
    
    return true;
}

void BorderEditorDialog::SaveGroundBrush() {
    if (!ValidateGroundBrush()) {
        return;
    }
    
    // Get the ground brush properties
    wxString name = m_nameCtrl->GetValue();
    
    // Double check that we have a name (it's also checked in ValidateGroundBrush)
    if (name.IsEmpty()) {
        wxMessageBox("You must provide a name for the ground brush.", "Error", wxICON_ERROR);
        return;
    }
    
    int serverId = m_serverLookIdCtrl->GetValue();
    int zOrder = m_zOrderCtrl->GetValue();
    int borderId = m_idCtrl->GetValue();  // This should be taken from common properties
    
    // Get the selected tileset
    int tilesetSelection = m_tilesetChoice->GetSelection();
    if (tilesetSelection == wxNOT_FOUND) {
        wxMessageBox("Please select a tileset.", "Validation Error", wxICON_ERROR);
        return;
    }
    wxString tilesetName = m_tilesetChoice->GetString(tilesetSelection);
    
    // Find the grounds.xml file using the same version path conversion
    wxString dataDir = g_gui.GetDataDirectory();
    
    // Get version string and convert to proper directory format
    wxString groundsFile = dataDir + wxFileName::GetPathSeparator() + 
                          "materials" + wxFileName::GetPathSeparator() + "brushs" + wxFileName::GetPathSeparator() + "grounds.xml";
    
    if (!wxFileExists(groundsFile)) {
        wxMessageBox("Cannot find grounds.xml file in the data directory.", "Error", wxICON_ERROR);
        return;
    }
    
    // Make sure the border is saved first if we have border items
    if (!m_borderItems.empty()) {
        SaveBorder();
    }
    
    // Load the XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(nstr(groundsFile).c_str());
    
    if (!result) {
        wxMessageBox("Failed to load grounds.xml: " + wxString(result.description()), "Error", wxICON_ERROR);
        return;
    }
    
    pugi::xml_node materials = doc.child("materials");
    if (!materials) {
        wxMessageBox("Invalid grounds.xml file: missing 'materials' node", "Error", wxICON_ERROR);
        return;
    }
    
    // Check if a brush with this name already exists
    bool brushExists = false;
    pugi::xml_node existingBrush;
    
    for (pugi::xml_node brushNode = materials.child("brush"); brushNode; brushNode = brushNode.next_sibling("brush")) {
        pugi::xml_attribute nameAttr = brushNode.attribute("name");
        
        if (nameAttr && wxString(nameAttr.as_string()) == name) {
            brushExists = true;
            existingBrush = brushNode;
            break;
        }
    }
    
    if (brushExists) {
        // Ask for confirmation to overwrite
        if (wxMessageBox("A ground brush with name '" + name + "' already exists. Do you want to overwrite it?", 
                        "Confirm Overwrite", wxYES_NO | wxICON_QUESTION) != wxYES) {
            return;
        }
        
        // Remove the existing brush
        materials.remove_child(existingBrush);
    }
    
    // Create the new brush node
    pugi::xml_node brushNode = materials.append_child("brush");
    brushNode.append_attribute("name").set_value(nstr(name).c_str());
    brushNode.append_attribute("type").set_value("ground");
    brushNode.append_attribute("server_lookid").set_value(serverId);
    brushNode.append_attribute("z-order").set_value(zOrder);
    
    // Add all ground items
    for (const GroundItem& item : m_groundItems) {
        pugi::xml_node itemNode = brushNode.append_child("item");
        itemNode.append_attribute("id").set_value(item.itemId);
        itemNode.append_attribute("chance").set_value(item.chance);
    }
    
    // Add border reference if we have border items, or if border ID is specified (even without items)
    if (!m_borderItems.empty() || borderId > 0) {
        // Get alignment type
        wxString alignmentType = m_borderAlignmentChoice->GetStringSelection();
        
        // Main border
        pugi::xml_node borderNode = brushNode.append_child("border");
        borderNode.append_attribute("align").set_value(nstr(alignmentType).c_str());
        borderNode.append_attribute("id").set_value(borderId);
        
        // "to none" border if checked
        if (m_includeToNoneCheck->IsChecked()) {
            pugi::xml_node borderToNoneNode = brushNode.append_child("border");
            borderToNoneNode.append_attribute("align").set_value(nstr(alignmentType).c_str());
            borderToNoneNode.append_attribute("to").set_value("none");
            borderToNoneNode.append_attribute("id").set_value(borderId);
        }
        
        // Inner border if checked
        if (m_includeInnerCheck->IsChecked()) {
            pugi::xml_node innerBorderNode = brushNode.append_child("border");
            innerBorderNode.append_attribute("align").set_value("inner");
            innerBorderNode.append_attribute("id").set_value(borderId);
            
            // Inner "to none" border if checked
            if (m_includeToNoneCheck->IsChecked()) {
                pugi::xml_node innerToNoneNode = brushNode.append_child("border");
                innerToNoneNode.append_attribute("align").set_value("inner");
                innerToNoneNode.append_attribute("to").set_value("none");
                innerToNoneNode.append_attribute("id").set_value(borderId);
            }
        }
    }
    
    // Save the file
    if (!doc.save_file(nstr(groundsFile).c_str())) {
        wxMessageBox("Failed to save changes to grounds.xml", "Error", wxICON_ERROR);
        return;
    }
    
    // Now also add this brush to the selected tileset
    wxString tilesetsFile = dataDir + wxFileName::GetPathSeparator() + 
                           "materials" + wxFileName::GetPathSeparator() + "tilesets.xml";
    
    if (!wxFileExists(tilesetsFile)) {
        wxMessageBox("Cannot find tilesets.xml file in the data directory.", "Error", wxICON_ERROR);
        return;
    }
    
    // Load the tilesets XML file
    pugi::xml_document tilesetsDoc;
    pugi::xml_parse_result tilesetsResult = tilesetsDoc.load_file(nstr(tilesetsFile).c_str());
    
    if (!tilesetsResult) {
        wxMessageBox("Failed to load tilesets.xml: " + wxString(tilesetsResult.description()), "Error", wxICON_ERROR);
        return;
    }
    
    pugi::xml_node tilesetsMaterials = tilesetsDoc.child("materials");
    if (!tilesetsMaterials) {
        wxMessageBox("Invalid tilesets.xml file: missing 'materials' node", "Error", wxICON_ERROR);
        return;
    }
    
    // Find the selected tileset
    bool tilesetFound = false;
    for (pugi::xml_node tilesetNode = tilesetsMaterials.child("tileset"); tilesetNode; tilesetNode = tilesetNode.next_sibling("tileset")) {
        pugi::xml_attribute nameAttr = tilesetNode.attribute("name");
        
        if (nameAttr && wxString(nameAttr.as_string()) == tilesetName) {
            // Find the terrain node
            pugi::xml_node terrainNode = tilesetNode.child("terrain");
            if (!terrainNode) {
                // Create terrain node if it doesn't exist
                terrainNode = tilesetNode.append_child("terrain");
            }
            
            // Check if the brush is already in this tileset
            bool brushFound = false;
            for (pugi::xml_node brushNode = terrainNode.child("brush"); brushNode; brushNode = brushNode.next_sibling("brush")) {
                pugi::xml_attribute brushNameAttr = brushNode.attribute("name");
                
                if (brushNameAttr && wxString(brushNameAttr.as_string()) == name) {
                    brushFound = true;
                    break;
                }
            }
            
            // If brush not found, add it
            if (!brushFound) {
                // Add a brush node directly under terrain - no empty attributes
                pugi::xml_node newBrushNode = terrainNode.append_child("brush");
                newBrushNode.append_attribute("name").set_value(nstr(name).c_str());
            }
            
            tilesetFound = true;
            break;
        }
    }
    
    if (!tilesetFound) {
        wxMessageBox("Selected tileset not found in tilesets.xml", "Error", wxICON_ERROR);
        return;
    }
    
    // Save the tilesets.xml file
    if (!tilesetsDoc.save_file(nstr(tilesetsFile).c_str())) {
        wxMessageBox("Failed to save changes to tilesets.xml", "Error", wxICON_ERROR);
        return;
    }
    
    wxMessageBox("Ground brush saved successfully and added to the " + tilesetName + " tileset.", "Success", wxICON_INFORMATION);
    
    // Reload the existing ground brushes list
    LoadExistingGroundBrushes();
}

void BorderEditorDialog::LoadTilesets() {
    // Clear the choice control
    m_tilesetChoice->Clear();
    m_tilesets.clear();
    
    // Find the tilesets.xml file
    wxString dataDir = g_gui.GetDataDirectory();
    
    // Get version string and convert to proper directory format
    wxString tilesetsFile = dataDir + wxFileName::GetPathSeparator() + 
                           "materials" + wxFileName::GetPathSeparator() + "tilesets.xml";
    
    if (!wxFileExists(tilesetsFile)) {
        wxMessageBox("Cannot find tilesets.xml file in the data directory.", "Error", wxICON_ERROR);
        return;
    }
    
    // Load the XML file
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(nstr(tilesetsFile).c_str());
    
    if (!result) {
        wxMessageBox("Failed to load tilesets.xml: " + wxString(result.description()), "Error", wxICON_ERROR);
        return;
    }
    
    pugi::xml_node materials = doc.child("materials");
    if (!materials) {
        wxMessageBox("Invalid tilesets.xml file: missing 'materials' node", "Error", wxICON_ERROR);
        return;
    }
    
    // Parse all tilesets
    wxArrayString tilesetNames; // Store in sorted order
    for (pugi::xml_node tilesetNode = materials.child("tileset"); tilesetNode; tilesetNode = tilesetNode.next_sibling("tileset")) {
        pugi::xml_attribute nameAttr = tilesetNode.attribute("name");
        
        if (nameAttr) {
            wxString tilesetName = wxString(nameAttr.as_string());
            
            // Add to our array of names
            tilesetNames.Add(tilesetName);
            
            // Add to the map for later use
            m_tilesets[tilesetName] = tilesetName;
        }
    }
    
    // Sort tileset names alphabetically
    tilesetNames.Sort();
    
    // Add sorted names to the choice control
    for (size_t i = 0; i < tilesetNames.GetCount(); ++i) {
        m_tilesetChoice->Append(tilesetNames[i]);
    }
    
    // Select the first tileset by default if any exist
    if (m_tilesetChoice->GetCount() > 0) {
        m_tilesetChoice->SetSelection(0);
    }
} 

// ============================================================================
// WallVisualPanel

BEGIN_EVENT_TABLE(WallVisualPanel, wxPanel)
    EVT_PAINT(WallVisualPanel::OnPaint)
END_EVENT_TABLE()

WallVisualPanel::WallVisualPanel(wxWindow* parent, wxWindowID id) :
    wxPanel(parent, id, wxDefaultPosition, wxSize(128, 128))    // 2x 64px tiles
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

WallVisualPanel::~WallVisualPanel() {
}

void WallVisualPanel::SetWallItems(const std::map<std::string, WallTypeData>& items) {
    m_items = items;
    Refresh();
}

void WallVisualPanel::Clear() {
    m_items.clear();
    Refresh();
}

void WallVisualPanel::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    
    // Draw background
    dc.SetBackground(wxBrush(wxColour(240, 240, 240)));
    dc.Clear();
    
    // Draw grid (2x2)
    int width = GetClientSize().GetWidth();
    int height = GetClientSize().GetHeight();
    int cellWidth = width / 2;
    int cellHeight = height / 2;
    
    dc.SetPen(wxPen(wxColour(200, 200, 200)));
    
    // Vertical line
    dc.DrawLine(cellWidth, 0, cellWidth, height);
    
    // Horizontal line
    dc.DrawLine(0, cellHeight, width, cellHeight);
    
    // Helper to draw the FIRST item of a type at a specific grid position (2x2)
    auto drawTypeAt = [&](const std::string& type, int gridX, int gridY) {
        if (m_items.find(type) != m_items.end() && !m_items[type].items.empty()) {
            uint16_t itemId = m_items[type].items[0].itemId; // Draw first item as preview
            if (itemId > 0) {
                int x = gridX * cellWidth;
                int y = gridY * cellHeight;
                
                const ItemType& itemType = g_items.getItemType(itemId);
                if (itemType.id != 0) {
                    Sprite* sprite = g_gui.gfx.getSprite(itemType.clientID);
                    if (sprite) {
                        sprite->DrawTo(&dc, SPRITE_SIZE_32x32, x, y, cellWidth, cellHeight);
                    }
                }
                
                // Draw label
                wxString label = wxString(type);
                wxSize textSize = dc.GetTextExtent(label);
                dc.SetTextForeground(*wxBLACK);
                dc.DrawText(label, x + 2, y + 2);
            }
        }
    };
    
    // Draw types in representative positions
    drawTypeAt("vertical", 0, 0);   // Top Left
    drawTypeAt("horizontal", 1, 0); // Top Right
    drawTypeAt("corner", 0, 1);     // Bottom Left
    drawTypeAt("pole", 1, 1);       // Bottom Right
} 
