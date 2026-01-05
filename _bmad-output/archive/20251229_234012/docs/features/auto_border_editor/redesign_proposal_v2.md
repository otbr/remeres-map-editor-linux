# Auto Border Editor: Redesign & UX Improvement Proposal (V2)

## 1. Core Insights & Problems Solved

The current implementation follows a standard "Form-based" approach (Tabs -> Inputs). The new vision shifts to a **"Workspace-based"** approach.

### Key UX Issues Addressed:
1.  **Visibility**: Currently, you select a border by name from a dropdown. *Insight*: Users should see *previews* of the border before selecting it.
2.  **Context Switching**: Users have to open a "Browse" modal to find IDs. *Insight*: An always-visible "Asset Palette" allows rapid experimentation via Drag & Drop.
3.  **Workflow**: Creating a new border currently requires manually entering IDs. *Insight*: Dragging a tile from the game assets directly onto the "North" edge of the grid is much faster (0 clicks vs 5 clicks).

## 2. Proposed Layout: The "Three-Panel" Workbench

We propose expanding the dialog into a larger, more comprehensive window (e.g., 900x600) with three distinct zones:

```text
+-----------------------------------------------------------------------+
|  Auto Border Editor                                               [X] |
+-----------------------------------------------------------------------+
| [ Toolbar: Save | New | Delete | Settings ]                           |
+-------------------+---------------------------+-----------------------+
|                   |                           |                       |
|  1. LIBRARY       |  2. WORKSPACE (Center)    |  3. ASSET PALETTE     |
|                   |                           |    (Right)            |
|  [ Search... ]    |  +---------------------+  |                       |
|                   |  |  Name: [ Mountain ] |  |  [ Search Items... ]  |
|  +-------------+  |  +---------------------+  |                       |
|  | [Preview]   |  |                           |  +-----------------+  |
|  | Mt. Edge    |  |       N   N   N           |  | [Sprite] [Sprite]| |
|  +-------------+  |     +---+---+---+         |  | ID:100   ID:101  | |
|  | [Preview]   |  |   W | 0 | 1 | 0 | E       |  |                  | |
|  | Grass       |  |     +---+---+---+         |  | [Sprite] [Sprite]| |
|  +-------------+  |     | 1 | X | 1 |         |  | ID:102   ID:103  | |
|  | [Preview]   |  |     +---+---+---+         |  |                  | |
|  | Cave        |  |     | 0 | 1 | 0 |         |  | ...              | |
|  +-------------+  |     +---+---+---+         |  |                  | |
|                   |       S   S   S           |  |                  | |
|                   |                           |  +-----------------+  |
|                   |  (Drop Targets Here)      |                       |
|                   |                           |  < Drag Sources >     |
|                   |  +---------------------+  |                       |
|                   |  | Live Preview        |  |                       |
|                   |  |                     |  |                       |
|                   |  | [  Rendered Result ]|  |                       |
|                   |  +---------------------+  |                       |
|                   |                           |                       |
+-------------------+---------------------------+-----------------------+
|  Status Bar: "Drag items from the right to the grid"                  |
+-----------------------------------------------------------------------+
```

### Module 1: The Visual Library (Left)
- **Function**: Replaces the "Load Existing" dropdown.
- **Component**: `wxListCtrl` or `wxTreeCtrl` with custom drawing.
- **Features**:
  - Displays a small thumbnail (32x32) of the border's "Center" or representative tile.
  - Search bar to filter by name (e.g., "snow").
  - Single-click to load into the Workspace.

### Module 2: The Workspace (Center)
- **Function**: The editing canvas.
- **Component**: Enhanced `BorderGridPanel`.
- **Drag & Drop Logic**:
  - The 3x3 Grid cells become `wxDropTarget`s.
  - **Behavior**: When a user drags an item from the Right Panel:
    - **Hover**: Is highlighted green.
    - **Drop**: Automatically extracts the Item ID and assigns it to that edge position (e.g., North).
- **Consolidation**: Properties (Group, Auto-Shape) are moved to a collapsible "Details" pane at the bottom or top of this column to maximize grid space.

### Module 3: The Asset Palette (Right) - *New Feature*
- **Function**: A "mini item browser" integrated directly into the editor.
- **Component**: A grid of `wxBitmapButton` or a custom `wxScrolledWindow` drawing sprites.
- **Features**:
  - **Tabs**: "Terrain", "Walls", "Doodads" (to filter relevant items).
  - **Drag Source**: Clicking and dragging an icon initiates a `wxTextDataObject` (containing the Item ID) drag operation.
  - **Search**: Filter items by name or ID.

## 3. Technical Implementation Strategy

### A. Drag and Drop (DnD)
Implement `wxTextDropTarget` on the `BorderGridPanel`.
```cpp
class BorderGridDropTarget : public wxTextDropTarget {
public:
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) {
        long itemId;
        if (data.ToLong(&itemId)) {
            // Calculate which grid cell (N, S, E, W...) is under (x,y)
            // Update the model with itemId
            // Refresh visuals
            return true;
        }
        return false;
    }
};
```

### B. Visual Library
Use a virtual `wxListCtrl` for performance if there are many borders.
- **Icon Generation**: On load, generate a temporary `wxBitmap` for each border by rendering its center/north pieces. Cache these in a `wxImageList`.

### C. Asset Reference
We need efficient access to `ItemType` or sprite data to populate the Right Panel. We can reuse the logic from `ItemSelector` or `BrowseTileWindow`, but embedded as a panel (`wxPanel`) instead of a modal dialog.

## 4. Benefits of this approach
1.  **Intuitive**: "I want *this* stone (drag) to be the *top* border (drop)."
2.  **Organized**: Separates "Choosing what to edit" (Left) from "Editing it" (Center) and "Finding parts" (Right).
3.  **Visual**: Minimizes text reading; maximizes visual recognition of tiles.
