# Palette XML Structure Analysis

This document analyzes the XML structure used by the map editor to define Borders, Grounds, and Walls. This information is crucial for implementing the "Auto Border Editor" feature to support creating and editing these types.

## File Hierarchy

 The main entry point is `data/materials/materials.xml`, which includes:
- `borders.xml` -> includes `borders/borders.xml`
- `brushs.xml` -> includes `brushs/grounds.xml`, `brushs/walls.xml`, and others.

## 1. Borders (`data/materials/borders/borders.xml`)

Borders define how tiles connect to each other at the edges.

### Structure
```xml
<border id="1" name="mountain edge border (new)">
    <borderitem edge="n" item="4445" />
    <borderitem edge="w" item="4448" />
    <!-- ... other edges ... -->
</border>
```

### Attributes
- **id**: Unique identifier for the border. Referenced by Ground brushes.
- **name**: Human-readable name (displayed in editor).
- **group**: (Optional) Group ID for categorization.
- **type**: (Optional) e.g., "optional" for optional borders.

### Elements
- **`<borderitem>`**: Defines the specific item ID for a given edge position.
    - **edge**: The position relative to the center tile.
        - Cardinals: `n`, `s`, `e`, `w`
        - Corners: `cnw`, `cne`, `cse`, `csw` (Inner corners?)
        - Diagonals: `dnw`, `dne`, `dse`, `dsw` (Outer corners?)
    - **item**: The server item ID (lookid) for the border piece.

## 2. Grounds (`data/materials/brushs/grounds.xml`)

Ground brushes define the base terrain tiles and how they interact with borders.

### Structure
```xml
<brush name="grass" type="ground" lookid="4515" z-order="3200">
    <item id="4515" chance="2500"/>
    <item id="4516" chance="10"/>
    
    <border align="outer" id="2"/>
    <border align="inner" to="none" id="1"/>
    <optional id="120"/>
</brush>
```

### Attributes
- **name**: Unique name of the brush (e.g., "grass").
- **type**: Must be "ground".
- **lookid**: The main item ID used for the palette icon.
- **z-order**: Rendering order (stacking priority).

### Elements
- **`<item>`**: Defines the items that make up the ground surface.
    - **id**: Item ID.
    - **chance**: Probability weight for randomization.
- **`<border>`**: links a Border definition to this ground.
    - **align**: "outer" or "inner".
    - **id**: Reference to `<border id="...">` in `borders.xml`.
    - **to**: (Optional) Specifies what this border connects *to* (e.g., "none", "sea", "ice").
- **`<optional>`**: Links to an optional border ID.
- **`<friend>`**: Declares another brush as a "friend" (connects seamlessly).
    - `name`: Name of the friend brush.

### Advanced: Specific Rules
For complex transitions (like "snow" vs "sea"), `<specific>` blocks are used:
```xml
<specific>
    <conditions>
        <match_border id="6" edge="s"/>
        <match_border id="5"  edge="e"/>
    </conditions>
    <actions>
        <replace_border id="5" edge="e" with="6652"/>
    </actions>
</specific>
```
- **conditions**: Logic to check neighboring borders/items.
- **actions**: Modifications to apply (replace items/borders) if conditions are met.

## 3. Walls (`data/materials/brushs/walls.xml`)

Wall brushes define vertical structures with variations for direction and interconnections.

### Structure
```xml
<brush name="stone wall" type="wall" server_lookid="1295">
    <wall type="horizontal">
        <item id="1295" chance="500"/>
        <door id="6251" type="normal" open="false"/>
        <!-- ... -->
    </wall>
    <wall type="vertical"> ... </wall>
    <wall type="corner"> ... </wall>
    <wall type="pole"> ... </wall>
</brush>
```

### Attributes
- **name**: Brush name.
- **type**: Must be "wall".
- **server_lookid**: Main item ID for the palette.

### Elements
- **`<wall>`**: Defines a specific orientation.
    - **type**:
        - `horizontal`: East-West wall.
        - `vertical`: North-South wall.
        - `corner`: Intersection/corner piece.
        - `pole`: Standalone column or post.
    - **`<item>`**: The wall item itself.
    - **`<door>`**: Defines doors compatible with this wall.
        - **id**: Item ID of the door.
        - **type**: "normal", "locked", "quest", "magic", "window", "hatch_window", "archway".
        - **open**: "true"/"false" state.

## Summary for Editor Implementation

To fully implement the Auto Border Editor for creating Grounds and Walls, we need to:

1.  **Grounds**:
    - Allow selecting a base set of items (with chances).
    - Allow adding "Border Layers" (Outer/Inner) by selecting from existing Borders (by ID).
    - Support defining "Friend" brushes.
    - (Advanced) UI for `<specific>` rules might be too complex for v1, but the structure exists.

2.  **Walls**:
    - Allow defining 4 distinct sets of items: Horizontal, Vertical, Corner, Pole.
    - Support adding Door IDs to each orientation.

3.  **Borders**:
    - We already have the structure (Center + 12 directions).
    - Need to ensure we can create a *new* Border definition and then immediately link it to a Ground being created.
