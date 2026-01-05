# Auto Border Editor Audit (Focus: Border Tab)

## 1. System Overview
*   **File**: `source/border_editor_window.cpp`
*   **Main Class**: `BorderEditorDialog` (Inherits `wxDialog`)
*   **Key Components**:
    *   `BorderGridPanel`: The 3x3 interactive grid (Left side).
    *   `BorderPreviewPanel`: The visual preview of the border (Right side).
    *   `m_borderItems`: Data vector storing `BorderItem` (position + item ID).

## 2. UI Architecture (The Modal)

The layout uses `wxBoxSizer` nesting. We have identified potential causes for "overlaps" and GTK warnings.

### Main Layout Hierarchy
```mermaid
graph TD
    Dialog[BorderEditorDialog (Vertical)]
    Dialog --> CommonProps[Common Properties (Vertical)]
    Dialog --> Notebook[wxNotebook]
    Notebook --> BorderTab[Border Panel (Vertical)]
    
    %% Common Properties
    CommonProps --> Row1[Row 1: Name, ID]
    CommonProps --> Row2[Row 2: Group, Checkboxes]

    %% Border Tab Content
    BorderTab --> Content[Content Sizer (Horizontal)]
    Content --> GridCol[Grid Column (Vertical, Flex=1)]
    Content --> PreviewCol[Preview Column (Vertical, Flex=1)]
    
    %% Grid Column Details
    GridCol --> GridPanel[BorderGridPanel (3x3)]
    GridCol --> Instructions[Text]
    GridCol --> ItemControls[Row: ID Input, Browse, Add]

    %% Preview Column Details
    PreviewCol --> PreviewPanel[BorderPreviewPanel]

    %% Bottom
    BorderTab --> Buttons[Save/Close Buttons]
```

### Critical Layout Issues (The Overlaps)
The GTK warnings (`for_size smaller than min-size`) indicate that containers are being forced smaller than their children's minimum requirements.

1.  **Dialog Constraint**: The dialog size is hardcoded to `wxSize(650, 520)`. This is likely too narrow for the side-by-side `GridCol` and `PreviewCol` plus margins on Linux/GTK.
2.  **SpinControl Issues**: `gtk_box_gadget_distribute: assertion 'size >= 0' failed` on `GtkSpinButton`. This often happens when a `wxSpinCtrl` is in a sizer without `wxEXPAND` or strict minimal width, and the parent squeezes it.
    *   *Suspects*: `m_itemIdCtrl` (Width 80), `m_idCtrl`, `m_groupCtrl`.
3.  **Horizontal Sizing**: `borderContentSizer` uses `wxEXPAND` on both children with proportion `1`. If the content of `GridPanel` or `PreviewPanel` enforces a minimum size (e.g., 32px sprites * 3 + padding), and the dialog width isn't sufficient, overlap occurs.

## 3. Data Flow
1.  **Loading**: `LoadExistingBorders` reads `data/materials/borders.xml`.
2.  **Selection**: User picks a border -> `OnLoadBorder` populates `m_borderItems`.
3.  **Interaction**:
    *   User clicks `BorderGridPanel`.
    *   `OnMouseClick` fires -> updates `m_selectedPosition`.
    *   User clicks "Add Manually" or uses Brush -> `m_borderItems` updated.
4.  **Rendering**: `UpdatePreview()` is called, triggering `BorderPreviewPanel::OnPaint` to draw sprites based on `m_borderItems`.

## 4. Logic & 12-Direction System
The editor relies on a mapping of 12 logical edge positions to coordinates.

*   **Enum**: `BorderEdgePosition` (n, e, s, w, cnw, cne, csw, cse, dnw, dne, dsw, dse).
*   **Visual Map**: These are mapped to a 3x3 grid (implied).
    *   `n` -> (1, 0)
    *   `w` -> (0, 1)
    *   `e` -> (2, 1)
    *   `s` -> (1, 2)
    *   *Corners* and *Diagonals* share specific mapped zones in the generic 3x3 grid logic, often leading to visual clustering (e.g., corners in corners).

## 5. Errors to Address
1.  **Fix Modal Width**: Increase purely hardcoded `(650, 520)` to `(800, 600)` or allow dynamic sizing based on sizer fitting (`GetSizer()->Fit(this)`).
2.  **Sizer Padding**: The warnings about "negative content height" suggest padding/margins are eating up all available space.
3.  **SpinCtrl Fixed Widths**: Removing hardcoded widths (e.g., `wxSize(80, -1)`) and using sizer proportions might fix the GTK assertion failures.

## 6. Prompting the AI
When asking the AI to fix this, specifically request:
*   "Refactor `CreateGUIControls` to ensure `wxSpinCtrl` elements have sufficient minimum size and aren't crushed by the fixed dialog width."
*   "Change the main Dialog constructor to use `SetSizerAndFit` or a larger default size."
*   "Review specific sizer flags for `m_itemIdCtrl` to prevent negative allocation."
