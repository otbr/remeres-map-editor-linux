# Technical Refactoring Blueprint: Auto Border Editor

## 1. Architectural Cleanup

### 1.1 The "Duplicate Panel" Logic
**Investigation Finding:**
The investigation confirmed that the code explicitly adds the **same pointer** (`m_borderPanel` and `m_groundPanel`) multiple times to the `wxNotebook`:

```cpp
// source/border_editor_window.cpp:521-526
m_notebook->AddPage(m_borderPanel, "Border");
m_notebook->AddPage(m_groundPanel, "Ground");
m_notebook->AddPage(m_borderPanel, "Border Loop"); // SAME POINTER
m_notebook->AddPage(m_groundPanel, "Ground Brush"); // SAME POINTER
```

**Risk Analysis:**
*   **Behavioral Undefinedness:** `wxNotebook` pages are expected to be unique windows. Sharing the same window instance causes undefined behavior regarding parenting, focus traversal, and visibility.
*   **Logic Errors:** The `OnSave` method checks `if (m_activeTab == 0)` for saving borders. If the user is on "Border Loop" (Index 2), the `else` block executes, triggering `SaveGroundBrush()`, which is incorrect for a border interface.

**Solution:**
1.  **Remove Duplicates:** Immediately remove the "Border Loop" and "Ground Brush" tabs if they are not distinct features.
2.  **Refactor if Needed:** If "Border Loop" is intended to be a different mode, it requires a unique logic class (e.g., `BorderLoopPanel`) or a flag mechanism, but it **must** have its own `wxPanel` instance. Given the current code, these appear to be vestigial or copy-paste errors. **Recommendation: Delete.**

### 1.2 Class Responsibility Violation
The `BorderEditorDialog` class handles logic for Borders, Grounds, and Walls all in one massive file (~2600 lines).

**Solution:**
*   **Extract Components:**
    *   `BorderPanel` (manages generic generic border logic)
    *   `GroundBrushPanel` (manages ground brush logic)
    *   `WallBrushPanel` (manages wall logic)
*   The Dialog should strictly act as a container and coordinator, not the implementation of every sub-feature.

## 2. UI/UX Modernization Plan

### 2.1 Hardcoded Constraints (Drawing)
The custom grid and preview panels rely heavily on "Retained Mode" drawing with hardcoded values in `OnPaint`.

**`BorderGridPanel::OnPaint`:**
*   **Cell Size:** `const int grid_cell_size = 64;` (Hardcoded local constant)
*   **Colors:** `wxColour(200, 200, 200)` (Background), `wxColour(100, 100, 100)` (Lines), `*wxRED_PEN` (Selection).
*   **Dimensions:** Layout logic (Normal, Corner, Diagonal) has fixed `offset_x/y` values (e.g., `10 + offset`).

**`BorderPreviewPanel::OnPaint`:**
*   **Grid Style:** `GRID_SIZE = 5` (Fixed).
*   **Colors:** Light gray background `(240, 240, 240)`.

**Optimization Strategy:**
1.  **Theme Constants:** Move colors to `static const wxColour` members or a `Theme` struct (e.g., `Theme::GridBackground`, `Theme::GridLines`).
2.  **Dynamic Sizing:** Calculate specific offsets based on `GetClientSize()` rather than fixed pixel margins, allowing the UI to scale.

### 2.2 Layout Flattening (The "Box" Problem)
The interface suffers from "Boxitis"—too many nested `wxStaticBoxSizer` elements creating visual clutter.

**Target for Removal/Replacement:**
1.  **`Common Properties` Box:** Remove the GroupBox border. Use a simple labeled separator (`wxStaticLine`) or just whitespace.
2.  **`Border Properties` Box:** Remove. The fields inside (Group, Type) can sit cleanly at the top of the specific tab.
3.  **`Border Grid` Box:** Remove. The grid is distinct enough visually; it doesn't need a frame.
4.  **`Preview` Box:** Remove.
5.  **`Ground Items` Box:** Remove.

**Resulting Hierarchy:**
*   (Dialog)
    *   Header (Name, ID)
    *   Separator
    *   Tabs
        *   Panel
            *   Top Controls (Flow Layout)
            *   Split Screen (Grid | Preview)
            *   Bottom Actions

## 3. Immediate Action Items

**Phase 1: Housekeeping (Safe Refactor)**
*   [ ] **Delete**: Remove lines adding "Border Loop" and "Ground Brush" tabs in `border_editor_window.cpp`.
*   [ ] **Clean**: Correct `OnSave` logic to switch on exact tab index or explicitly check panel type.

**Phase 2: Visual De-clutter (High Impact)**
*   [ ] **Refactor Sizers**: Replace `wxStaticBoxSizer` with `wxBoxSizer` in `CreateGUIControls`.
*   [ ] **Styling**: Replace `wxBLUE`/`wxRED` text colors with `wxSYS_COLOUR_HIGHLIGHT` or similar system constants for native look.

**Phase 3: Drawing Modernization (High Effort)**
*   [ ] **Refactor `OnPaint`**: Extract colors to class constants.
*   [ ] **Refactor `OnPaint`**: Replace magic numbers (`64`, `10`, `20`) with named constants (`CELL_SIZE`, `MARGIN`, `SECTION_SPACING`).

## Approval Request
Please approve this blueprint to begin with **Phase 1: Housekeeping**.
