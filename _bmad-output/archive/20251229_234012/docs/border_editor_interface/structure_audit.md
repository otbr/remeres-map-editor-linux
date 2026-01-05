# Auto Border Editor UI Audit

## 1. Overview
The **Auto Border Editor** is a modal dialog (`wxDialog`) designed to create and edit border brushes and ground brushes.
*   **File**: `source/border_editor_window.cpp`
*   **Class**: `BorderEditorDialog`
*   **Base Dimensions**: 850x650 pixels (Hardcoded)
*   **Style**: `wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER`

## 2. High-Level Structure
The interface is constructed using a vertical `wxBoxSizer` containing:
1.  **Common Properties**: A shared header section.
2.  **Tab Control (`wxNotebook`)**: The main content area with multiple tabs.

### 2.1 Common Properties
*   **Container**: `wxStaticBoxSizer` (Vertical, Label: "Common Properties")
*   **Layout**: `wxBoxSizer` (Horizontal)
*   **Elements**:
    *   **Name**: `wxTextCtrl` (Expandable)
    *   **ID**: `wxSpinCtrl` (Range 1-1000)
    *   **Padding**: `wxEXPAND | wxRIGHT` 10px spacing between name and ID.

## 3. Tab Structure
The `wxNotebook` contains the following pages:

| Tab Name | Panel Variable | Description |
| :--- | :--- | :--- |
| **Border** | `m_borderPanel` | Main border editing interface. |
| **Ground** | `m_groundPanel` | Ground brush configuration. |
| **Border Loop** | `m_borderPanel` | *Reuses Border Panel*. Logic implies duplicate or placeholder. |
| **Ground Brush** | `m_groundPanel` | *Reuses Ground Panel*. Logic implies duplicate or placeholder. |
| **Wall** | `m_wallPanel` | Wall brush structure editor. |

### 3.1 Border Tab (`m_borderPanel`)
**Layout Strategy**: Vertical Stack.

1.  **Border Properties** (`wxStaticBoxSizer`)
    *   **Left Column**:
        *   **Group**: `wxSpinCtrl`
        *   **Type**: Checkboxes (`Optional`, `Ground`)
    *   **Right Column**:
        *   **Load Existing**: `wxComboBox` (ReadOnly)

2.  **Content Area** (Horizontal Split 50/50 approx)
    *   **Left: Grid Editor** (`BorderGridPanel`)
        *   Custom drawn grid (3 sections: Simple, Corner, Diagonal).
        *   **Interaction**: Click to select position.
        *   **Controls**: Item ID input, "Browse...", "Add Manually".
        *   **Instructions**: Static text (Blue).
    *   **Right: Preview** (`BorderPreviewPanel`)
        *   Custom drawn preview (192x192px).
        *   Draws central ground tile + surrounding border items.

3.  **Action Buttons** (Bottom)
    *   Clear, Save Border, Close.

### 3.2 Ground Tab (`m_groundPanel`)
**Layout Strategy**: Vertical Stack.

1.  **Ground Brush Properties** (`wxStaticBoxSizer`)
    *   **Row 1**: Tileset (`wxChoice`), Server Look ID (`wxSpinCtrl`).
    *   **Row 2**: Z-Order (`wxSpinCtrl`), Load Existing (`wxComboBox`).

2.  **Ground Items** (`wxStaticBoxSizer`)
    *   **List**: `wxListBox` (Height: 100px).
    *   **Controls**: Item ID, Chance inputs, Add/Remove/Browse buttons.

3.  **Border for Ground Brush** (`wxStaticBoxSizer`)
    *   **Border Alignment**: `wxChoice` (Outer, Inner).
    *   **Options**: `To None`, `Inner Border` checkboxes.
    *   **Instruction**: Static text (Blue) pointing to Border tab.
    *   **Warning**: Static text (Red) about ID usage.

### 3.3 Wall Tab (`m_wallPanel`)
**Layout Strategy**: Vertical Stack.

1.  **Top Section**:
    *   Load Existing (`wxComboBox`), Server Look ID (`wxSpinCtrl`).
2.  **Content Area** (Horizontal):
    *   **Left: Structure Editor**:
        *   Logic Type (`wxChoice`: vertical, horizontal, etc.).
        *   Items List (`wxListBox`).
        *   Add/Remove controls.
    *   **Right: Preview** (`WallVisualPanel`).
3.  **Buttons**: Clear All, Save Wall, Close.

## 4. Aesthetic & Technical Observations

### 4.1 Spacing & Alignment
*   **Margins**: `wxALL | 5` is used universally. This creates a functional but "cramped" look with 5px gaps everywhere.
*   **Sizing**: Many controls rely on `wxDefaultSize`.
*   **Inputs**: `wxSpinCtrl` and `wxTextCtrl` heights may not align perfectly depending on OS theme.

### 4.2 Colors & Styling
*   **Helpers**: Uses raw `*wxBLUE` and `*wxRED` for instructional/warning text.
*   **Grid Panels**:
    *   `BorderGridPanel`: Gray background (200,200,200), dark gray lines (100,100,100).
    *   `BorderPreviewPanel`: Lighter gray background (240,240,240), light grid lines. Green center tile (120,180,100).
*   **Consistency**: No central theme or palette used; values are hardcoded in `OnPaint` methods.

### 4.3 Structure Issues
*   **Panel Reuse**: "Border Loop" and "Ground Brush" tabs reusing pointer instances (`m_borderPanel`, `m_groundPanel`) is highly unusual and likely causes state conflicts or is a placeholder implementation.
*   **Visual Hierarchy**: `wxStaticBoxSizer` (Group Boxes) are heavily used to delineate sections, adding many borders within borders.

## 5. Improvement Recommendations
1.  **Framing**: Remove nested Group Boxes in favor of clean headers or spacing to reduce visual noise.
2.  **Consistency**: define color constants for the grid/preview panels instead of hardcoded RGB.
3.  **Dimensions**: Remove hardcoded `850x650` if possible, or adjust for better breathing room.
4.  **Tab Logic**: Clarify the purpose of duplicate tabs. If they are distinct modes, they need distinct (or properly reset) UI instances.
