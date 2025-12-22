// border_editor_window.h - Provides a visual editor for auto borders

#ifndef RME_BORDER_EDITOR_WINDOW_H_
#define RME_BORDER_EDITOR_WINDOW_H_

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/spinctrl.h>
#include <wx/bmpbuttn.h>
#include <wx/artprov.h>
#include <wx/grid.h>
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/notebook.h>
#include <wx/simplebook.h>
#include <wx/tglbtn.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>
#include <vector>
#include <map>
#include <set>
#include <wx/treectrl.h>
#include <wx/checklst.h>

// Forward declarations
class BorderItemButton;
class BorderGridPanel;
class GroundGridPanel;
class GroundGridContainer;
class BorderPreviewPanel;
class WallVisualPanel;
class BrushPalettePanel;

// Represents a border edge position
enum BorderEdgePosition {
    EDGE_NONE = -1,
    EDGE_N,   // North
    EDGE_E,   // East
    EDGE_S,   // South
    EDGE_W,   // West
    EDGE_CNW, // Corner Northwest
    EDGE_CNE, // Corner Northeast
    EDGE_CSE, // Corner Southeast
    EDGE_CSW, // Corner Southwest
    EDGE_DNW, // Diagonal Northwest
    EDGE_DNE, // Diagonal Northeast
    EDGE_DSE, // Diagonal Southeast
    EDGE_DSW, // Diagonal Southwest
    EDGE_COUNT
};

// Alignment options for borders
enum BorderAlignment {
    ALIGN_OUTER,
    ALIGN_INNER
};

// Utility function to convert border edge string to position
BorderEdgePosition edgeStringToPosition(const std::string& edgeStr);

// Utility function to convert border position to string
std::string edgePositionToString(BorderEdgePosition pos);

// Represents a border item
struct BorderItem {
    BorderEdgePosition position;
    uint16_t itemId;
    
    BorderItem() : position(EDGE_NONE), itemId(0) {}
    BorderItem(BorderEdgePosition pos, uint16_t id) : position(pos), itemId(id) {}
    
    bool operator==(const BorderItem& other) const {
        return position == other.position && itemId == other.itemId;
    }
    
    bool operator!=(const BorderItem& other) const {
        return !(*this == other);
    }
};

// Represents a ground item with chance
struct GroundItem {
    uint16_t itemId;
    int chance;
    
    GroundItem() : itemId(0), chance(10) {}
    GroundItem(uint16_t id, int c) : itemId(id), chance(c) {}
    
    bool operator==(const GroundItem& other) const {
        return itemId == other.itemId && chance == other.chance;
    }
    
    bool operator!=(const GroundItem& other) const {
        return !(*this == other);
    }
};

// Represents a wall type group (e.g., "vertical", "horizontal")
struct WallTypeData {
    std::string typeName;
    std::vector<GroundItem> items; // Reuse GroundItem as it has {itemId, chance}
};

// Represents a wall item position
struct WallItem {
    int x; // Grid X (0-2)
    int y; // Grid Y (0-2)
    uint16_t itemId;
    
    WallItem() : x(-1), y(-1), itemId(0) {}
    WallItem(int _x, int _y, uint16_t id) : x(_x), y(_y), itemId(id) {}
};

// Tree item data to store asset info
class BorderAssetItemData : public wxTreeItemData {
public:
    enum AssetType {
        FOLDER,
        BORDER,
        GROUND,
        WALL
    };
    
    BorderAssetItemData(AssetType type, int id = 0, const wxString& name = "") 
        : m_type(type), m_id(id), m_name(name) {}
        
    AssetType GetType() const { return m_type; }
    int GetId() const { return m_id; }
    wxString GetName() const { return m_name; }
    
private:
    AssetType m_type;
    int m_id;
    wxString m_name;
};

// Represents a single tile in a composite variation
struct CompositeTile {
    int x, y;
    uint16_t itemId;

    bool operator<(const CompositeTile& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

// Represents a variation (composite block)
struct GroundVariation {
    int chance;
    std::vector<CompositeTile> tiles;
    
    GroundVariation() : chance(10) {}
};

class SimpleRawPalettePanel;

// Dialog for filtering visible tilesets
class TilesetFilterDialog : public wxDialog {
public:
    TilesetFilterDialog(wxWindow* parent, 
                       const wxArrayString& allTilesets,
                       const std::set<wxString>& enabledTilesets,
                       const wxString& modeLabel);
    
    std::set<wxString> GetEnabledTilesets() const;
    
private:
    wxCheckListBox* m_tilesetList;
    wxArrayString m_allTilesets;
    
    void OnSelectAll(wxCommandEvent& event);
    void OnSelectNone(wxCommandEvent& event);
    void OnListItemClick(wxCommandEvent& event);
};

class BorderEditorDialog : public wxDialog {
public:
    BorderEditorDialog(wxWindow* parent, const wxString& title);
    virtual ~BorderEditorDialog();

    // Event handlers
    void OnItemIdChanged(wxCommandEvent& event);
    void OnPositionSelected(wxCommandEvent& event);
    void OnAddItem(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnBrowse(wxCommandEvent& event);
    void OnLoadBorder(wxCommandEvent& event);
    void OnGridCellClicked(wxMouseEvent& event);
    void OnPageChanged(wxBookCtrlEvent& event);
    void OnAddGroundItem(wxCommandEvent& event);
    void OnRemoveGroundItem(wxCommandEvent& event);
    void OnGroundBrowse(wxCommandEvent& event);
    void OnGroundTilesetSelect(wxCommandEvent& event);
    void OnGroundPaletteSelect(wxCommandEvent& event);
    void OnOpenTilesetFilter(wxCommandEvent& event);
    
    // Wall events
    void OnWallBrowse(wxCommandEvent& event);
    void OnGroundTilesetListSelect(wxCommandEvent& event);
    void OnRawCategoryChange(wxCommandEvent& event);
    void OnAddWallItem(wxCommandEvent& event);
    void OnRemoveWallItem(wxCommandEvent& event);
    
    // Sidebar events
    void OnAssetTreeSelection(wxTreeEvent& event);
    void OnAssetSearch(wxCommandEvent& event);
    
    // Browser sidebar events
    void OnBorderBrowserSelection(wxCommandEvent& event);
    void OnBrowserSearch(wxCommandEvent& event);
    
    // Mode switcher events
    void OnModeSwitch(wxCommandEvent& event);

protected:
    void CreateGUIControls();
    void LoadExistingBorders();
    void LoadExistingGroundBrushes();
    void LoadTilesets();
    void SaveBorder();
    void SaveGroundBrush();
    bool ValidateBorder();
    void LoadBorderById(int borderId);
    void LoadGroundBrushByName(const wxString& name);
    void LoadWallBrushByName(const wxString& name);
    bool ValidateGroundBrush();
    bool ValidateWallBrush();
    void LoadExistingWallBrushes();
    void SaveWallBrush();
    void ClearWallItems();
    void UpdateWallItemsList();
    void UpdatePreview();
    void ClearItems();
    void ClearGroundItems();
    void UpdateGroundItemsList();
    
    // Sidebar helpers
    void PopulateAssetTree();
    void FilterAssetTree(const wxString& query);
    
    // Browser population methods
    void PopulateBorderList();
    void PopulateGroundList();
    void PopulateWallList();
    void FilterBrowserList(const wxString& query);
    void UpdateBrowserLabel();

private:
    // ===== State =====
    int m_maxBorderId;
    int m_nextBorderId;
    int m_currentBorderId;  // Currently edited border ID (replaces m_idCtrl)
    int m_activeTab;        // 0 = border, 1 = ground, 2 = wall
    
    // Storage for unfiltered browser list items
    wxArrayString m_fullBrowserList;
    wxArrayString m_fullBrowserIds;
    
    // Animation timer for preview
    wxTimer* m_previewTimer;

    // ===== UI Controls =====
    wxSimplebook* m_notebook;
    
    // Mode switcher buttons
    wxToggleButton* m_borderModeBtn;
    wxToggleButton* m_groundModeBtn;
    wxToggleButton* m_wallModeBtn;
    
    // Common controls
    wxTextCtrl* m_nameCtrl;
    wxTextCtrl* m_groundNameCtrl;
    wxButton* m_newButton;  // "New Border/Ground/Wall" button with dynamic label
    
    // Sidebar (Tree view - legacy, may not be used)
    wxTextCtrl* m_searchCtrl;
    wxTreeCtrl* m_assetTree;
    wxTreeItemId m_rootId;
    wxTreeItemId m_bordersRootId;
    wxTreeItemId m_groundsRootId;
    wxTreeItemId m_wallsRootId;
    
    // ===== Border Tab =====
    wxPanel* m_borderPanel;
    wxListBox* m_borderBrowserList;     // Sidebar browser for existing items
    wxSearchCtrl* m_browserSearchCtrl;  // Search control for sidebar
    wxStaticText* m_browserLabel;       // Dynamic label for sidebar
    wxCheckBox* m_isOptionalCheck;
    wxCheckBox* m_isGroundCheck;
    wxCheckBox* m_groupCheck;           // NEW: Replaces m_groupCtrl SpinCtrl
    BorderGridPanel* m_gridPanel;
    BorderPreviewPanel* m_previewPanel;
    // Palette panels
    SimpleRawPalettePanel* m_itemPalettePanel;  // Changed from BrushPalettePanel*
    SimpleRawPalettePanel* m_groundPalette;     
    GroundGridContainer* m_groundGridContainer;
    BrushPalettePanel* m_wallPalette;
    
    // Border items data
    std::vector<BorderItem> m_borderItems;
    std::map<BorderEdgePosition, BorderItemButton*> m_borderButtons;
    
    // ===== Ground Tab =====
    wxPanel* m_groundPanel;
    wxSpinCtrl* m_serverLookIdCtrl;
    wxSpinCtrl* m_zOrderCtrl;
    wxSpinCtrl* m_groundItemIdCtrl;
    wxSpinCtrl* m_groundItemChanceCtrl;
    wxListBox* m_groundItemsList;
    wxChoice* m_borderAlignmentChoice;
    wxCheckBox* m_includeToNoneCheck;

    wxCheckBox* m_includeInnerCheck;
    wxComboBox* m_groundTilesetCombo;
    wxArrayString m_tilesetListData;
    wxComboBox* m_rawCategoryCombo;
    
    // Ground items data
    std::vector<GroundItem> m_groundItems;
    std::map<wxString, wxString> m_tilesets;
    long long m_lastInteractionTime;
    
    // Composite/Variation support
    std::vector<GroundVariation> m_groundVariations;
    int m_currentVariationIndex;
    
    void UpdateVariationControls();
    void RenderCurrentVariation();
    
    // Composite/Variation support
    // UI for Scrollable List of Variations
    wxScrolledWindow* m_variationsScrolledWindow;
    wxBoxSizer* m_variationsSizer;
    class VariationPreviewPanel* m_variationPreview;
    wxButton* m_addVariationBtn;

    // Helper to rebuild the list UI from m_groundVariations
    void RebuildVariationsList();
    void OnRemoveVariationBtn(wxCommandEvent& event); // Handle removal from specific row
    
    // Internal helper to add one UI row
    void AddVariationRow(GroundVariation& variation, int index);
    
    void OnPreviewTimer(wxTimerEvent& event);
    
    void OnAddVariation(wxCommandEvent& event);
    
    // Tileset filter whitelists (per-mode) - only enabled tilesets
    std::set<wxString> m_borderEnabledTilesets;
    std::set<wxString> m_groundEnabledTilesets;
    std::set<wxString> m_wallEnabledTilesets;
    wxBitmapButton* m_tilesetFilterBtn;  // Filter button
    
    // Filter config persistence
    void SaveFilterConfig();
    void LoadFilterConfig();

    // BrushPalettePanel* m_groundPalette;  // Removed duplicate
    
    // ===== Wall Tab =====
    wxPanel* m_wallPanel;
    wxSpinCtrl* m_wallServerLookIdCtrl;
    wxSpinCtrl* m_wallGroupCtrl;
    wxSpinCtrl* m_wallZOrderCtrl;
    wxSpinCtrl* m_wallItemIdCtrl;
    wxSpinCtrl* m_wallItemChanceCtrl;
    wxListBox* m_wallItemsList;
    wxCheckBox* m_wallIsOptionalCheck;
    wxCheckBox* m_wallIsGroundCheck;
    WallVisualPanel* m_wallVisualPanel;
    wxChoice* m_wallTypeChoice;
    
    // Wall items data
    // Wall items data
    std::map<std::string, WallTypeData> m_wallTypes;
    wxString m_currentWallType;
    // BrushPalettePanel* m_wallPalette;  // Removed duplicate
    
    DECLARE_EVENT_TABLE()
};

// Custom button to represent a border item
class BorderItemButton : public wxButton {
public:
    BorderItemButton(wxWindow* parent, BorderEdgePosition position, wxWindowID id = wxID_ANY);
    virtual ~BorderItemButton();
    
    void SetItemId(uint16_t id);
    uint16_t GetItemId() const { return m_itemId; }
    BorderEdgePosition GetPosition() const { return m_position; }
    
    void OnPaint(wxPaintEvent& event);
    
private:
    uint16_t m_itemId;
    BorderEdgePosition m_position;
    
    DECLARE_EVENT_TABLE()
};

// Grid panel to visually show border item positions
class SimpleRawPalettePanel : public wxScrolledWindow {
public:
    SimpleRawPalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~SimpleRawPalettePanel();

    void LoadTileset(const wxString& categoryName);
    void SetItemIds(const std::vector<uint16_t>& ids);
    void OnPaint(wxPaintEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnLeave(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);

private:
    std::vector<uint16_t> m_itemIds;
    int m_cols = 1;
    int m_rows = 1;
    int m_itemSize = 32;
    int m_padding = 2;
    int m_hoverIndex = -1;

    void RecalculateLayout();
    int HitTest(const wxPoint& pt) const;

    DECLARE_EVENT_TABLE()
};

// Grid panel to visually show border item positions
class BorderGridPanel : public wxPanel {
public:
    BorderGridPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~BorderGridPanel();
    
    void SetItemId(BorderEdgePosition pos, uint16_t itemId);
    uint16_t GetItemId(BorderEdgePosition pos) const;
    void Clear();
    
    // Selection management
    void SetSelectedPosition(BorderEdgePosition pos);
    BorderEdgePosition GetSelectedPosition() const { return m_selectedPosition; }
    
    // Override to ensure correct sizing
    virtual wxSize DoGetBestSize() const override;
    
    void OnPaint(wxPaintEvent& event);
    void OnMouseClick(wxMouseEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    
    // Made public so it can be accessed from other components
    wxPoint GetPositionCoordinates(BorderEdgePosition pos) const;
    BorderEdgePosition GetPositionFromCoordinates(int x, int y) const;
    
private:
    std::map<BorderEdgePosition, uint16_t> m_items;
    BorderEdgePosition m_selectedPosition;
    
    DECLARE_EVENT_TABLE()
};

// Panel to preview how the border would look
class BorderPreviewPanel : public wxPanel {
public:
    BorderPreviewPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~BorderPreviewPanel();
    
    void SetBorderItems(const std::vector<BorderItem>& items);
    void Clear();
    
    void OnPaint(wxPaintEvent& event);
    
private:
    std::vector<BorderItem> m_borderItems;
    
    DECLARE_EVENT_TABLE()
};

// Panel to visually edit wall brushes
class WallVisualPanel : public wxPanel {
public:
    WallVisualPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~WallVisualPanel();
    
    void SetWallItems(const std::map<std::string, WallTypeData>& items);
    void Clear();
    
    void OnPaint(wxPaintEvent& event);
    
private:
    std::map<std::string, WallTypeData> m_items;
    
    DECLARE_EVENT_TABLE()
};



// ============================================================================
// GroundGridPanel (Visual Replica of BorderGridPanel)
// ============================================================================
class GroundGridPanel : public wxPanel {
public:
    GroundGridPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~GroundGridPanel();
    
    // Item management
    void SetItemId(uint16_t id);
    uint16_t GetItemId() const { return m_itemId; }
    void Clear();
    bool IsEmpty() const { return m_itemId == 0; }
    
    // Event handlers
    void OnPaint(wxPaintEvent& event);
    void OnMouseClick(wxMouseEvent& event);
    
    // Layout helper
    wxSize DoGetBestSize() const override;

private:
    uint16_t m_itemId;  // Currently assigned item ID
    
    DECLARE_EVENT_TABLE()
};

// ============================================================================
// GroundGridContainer - Manages multiple GroundGridPanel cells with auto-expansion
// ============================================================================
class GroundGridContainer : public wxScrolledWindow {
public:
    GroundGridContainer(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~GroundGridContainer();
    
    // Item management
    void AddItem(uint16_t itemId);
    void Clear();
    
    // Get all filled items
    std::vector<uint16_t> GetAllItems() const;
    size_t GetFilledCount() const;
    
    // Called when a cell is filled - auto-adds new empty cell
    // Called when a cell is filled - auto-adds new empty cell
    void OnCellFilled(int index);
    
    // Make sure there's always one empty cell at the end
    void EnsureEmptyCell();
    
    // Rebuild grid layout after changes
    void RebuildGrids();

private:
    std::vector<GroundGridPanel*> m_gridCells;
    wxBoxSizer* m_gridSizer;
    
    void CreateNewCell();          // Create a new grid cell
};


#endif // RME_BORDER_EDITOR_WINDOW_H_