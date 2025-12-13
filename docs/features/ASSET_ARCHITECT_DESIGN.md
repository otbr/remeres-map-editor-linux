# Asset Architect: Design & Roadmap

## 1. Concept
The **Auto Border Editor** is evolving into the **Asset Architect** (or "Asset Visualizer").
**Goal:** Provide a unified interface to **Visualize, Edit, and Create** all procedural map assets (`borders`, `grounds`, `walls`, `doodads`), removing the mysteries of XML editing.

## 2. Asset Registration Logic (The Flowchart)
Each asset type follows a specific "Logic Driver" that determines how it is placed on the map.

```mermaid
graph TD
    subgraph "Global Data Sources"
        XM[XML Files] -->|Parse| G[Global Asset Dictionaries]
        G --> B[Borders Dict]
        G --> GR[Grounds Dict]
        G --> W[Walls Dict]
        G --> D[Doodads Dict]
    end

    subgraph "1. Borders (Connectivity Logic)"
        B -->|Defined in| File1[borders.xml]
        File1 -->|Structure| BStruct
        BStruct("Border ID") -->|Contains| BItems
        BItems{Positions}
        BItems -->|Edge: N| ItemN[ItemID 100]
        BItems -->|Edge: S| ItemS[ItemID 101]
        BItems -->|Corner: CNW| ItemCNW[ItemID 102]
        
        style BStruct fill:#f9f,stroke:#333
    end

    subgraph "2. Grounds (Base + Reference Logic)"
        GR -->|Defined in| File2[grounds.xml]
        File2 -->|Structure| GRStruct
        GRStruct("Brush Name") -->|Contains| GRItems
        GRItems -->|Base| RandomItems[Random Items (ID + Chance)]
        GRItems -->|Reference| BorderRef[Border ID or Alignment]
        BorderRef -.->|Links to| BStruct
        
        style GRStruct fill:#bbf,stroke:#333
    end

    subgraph "3. Walls (Directional Logic)"
        W -->|Defined in| File3[walls.xml]
        File3 -->|Structure| WStruct
        WStruct("Brush Name") -->|Contains| WallTypes
        WallTypes -->|Type: Horizontal| W_H[Items + Doors]
        WallTypes -->|Type: Vertical| W_V[Items + Doors]
        WallTypes -->|Type: Corner| W_C[Items]
        WallTypes -->|Type: Pole| W_P[Items]
        
        style WStruct fill:#dfd,stroke:#333
    end

    subgraph "4. Doodads (Random/Composite Logic)"
        D -->|Defined in| File4[doodads.xml]
        File4 -->|Structure| DStruct
        DStruct("Brush Name") -->|Type: Doodad| D_Simple
        D_Simple -->|Random| List[Item List (Chance)]
        D_Simple -->|Composite| Comp[Multi-Tile Structure]
        
        DStruct -->|Type: Carpet| D_Carpet
        D_Carpet -->|Align: N/S/E/W| CarpetLogic[Uses Border-like Logic]
        
        style DStruct fill:#ff9,stroke:#333
    end
```

### Key Takeaways for Tool Expansion:
1.  **Borders & Carpets**: Share the same `N/S/E/W` logic. Can reuse the current Grid Visualizer.
2.  **Grounds**: Are "Wrappers" around a list of items + a pointer to a Border. Current tool already handles this well.
3.  **Walls**: Introduce **"Type" logic** (Vertical vs Horizontal). The Visualizer needs tabs or a different grid layout (a "Cross" shape would be ideal to show corners and sides).
4.  **Doodads**: Require a "Canvas" preview since they can be multi-tile composites.

## 3. Implementation Roadmap

We will transform the current "Auto Border Editor" into the "Asset Architect" in phases.

### Phase 1: The "Visualizer" Transformation (Current Focus)
*Goal: Make the current tool intuitive for exploring existing assets.*
- [ ] **UI Refactor**: Replace "Load Existing" dropdown with a **Side Panel Explorer** (Tree View).
- [ ] **Search**: Add a filter box to find "grass", "mountain", etc.
- [ ] **Mode Switch**: Toggle between "View Mode" (Safe, No formatting) and "Edit Mode" (Unlocked).
- [ ] **Smart Context**: When clicking a visual tile, jump to that "Edge" definition.

### Phase 2: Walls Editor
*Goal: Expand support to `walls.xml`.*
- [ ] **Parsing**: Add generic parser for `walls.xml`.
- [ ] **UI Update**: Add "Walls" tab.
- [ ] **Visualization**: Create a "Wall Preview" grid (showing a corner connection: Vertical Wall meeting Horizontal Wall).
- [ ] **Metadata**: Handle "Doors" and "Friends" (related walls).

### Phase 3: Doodads & Objects Architect
*Goal: Support `doodads.xml`.*
- [ ] **Parsing**: Handle `composite` and `random` types.
- [ ] **Visualization**: 
    -   Simple items: Show grid of variations.
    -   Carpets: Reuse Border Grid.
    -   Composites: Show mini-map preview.

### Phase 4: Integration
*Goal: Deep integration with the Map Editor.*
- [ ] **Right-Click Context**: Right-click a tile in the map -> "Edit this Asset" opens the Architect with that asset loaded.
- [ ] **Hot-Reload**: Saving an asset immediately updates the palette in the main editor.
