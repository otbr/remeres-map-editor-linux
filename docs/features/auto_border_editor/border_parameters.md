# Auto Border Parameters & XML Structure

This document details the structure of the `borders.xml` file and the parameters used to define auto-border logic.

## 1. XML Structure

The borders are defined in `data/materials/borders.xml` (which typically includes `data/materials/borders/borders.xml`).

### Schema
```xml
<materials>
    <border id="[INTEGER]" name="[STRING]" group="[INTEGER]" type="[STRING]">
        <borderitem edge="[EDGE_ID]" item="[ITEM_ID]"/>
        <!-- ... multiple borderitems ... -->
    </border>
</materials>
```

### Attributes
- **id** (Required): A unique integer identifier for the border set.
- **name** (Optional): A descriptive name for the border.
- **group** (Optional): Group ID. Borders with the same group ID will connect to each other.
- **type** (Optional): Special behavior flags, e.g., `optional`.

## 2. Edge Parameters (The 12 Directions)

The auto-border system uses a 12-bit bitmask logic represented by specific edge codes. These define which tile should be placed based on the adjacency of neighbors.

### Cardinal Directions (Straight Edges)
| Edge Code | Direction | Description |
|-----------|-----------|-------------|
| `n` | **North** | Top Center tile. |
| `e` | **East** | Right Center tile. |
| `s` | **South** | Bottom Center tile. |
| `w` | **West** | Left Center tile. |

### Inner Corners (Concave)
Refers to "Corners" where the terrain turns inward.
| Edge Code | Direction | Description |
|-----------|-----------|-------------|
| `cnw` | **Corner North-West** | Top-Left Inner Corner. |
| `cne` | **Corner North-East** | Top-Right Inner Corner. |
| `cse` | **Corner South-East** | Bottom-Right Inner Corner. |
| `csw` | **Corner South-West** | Bottom-Left Inner Corner. |

### Diagonal/Outer Corners (Convex)
Refers to "Diagonals" where the terrain turns outward (tips).
| Edge Code | Direction | Description |
|-----------|-----------|-------------|
| `dnw` | **Diagonal North-West** | Top-Left Outer Tip. |
| `dne` | **Diagonal North-East** | Top-Right Outer Tip. |
| `dse` | **Diagonal South-East** | Bottom-Right Outer Tip. |
| `dsw` | **Diagonal South-West** | Bottom-Left Outer Tip. |

## 3. Visual Reference (Grid Mapping)

The Auto Border Editor represents these positions in a 3x3 grid (excluding the center):

```text
+-------+-------+-------+
|  DNW  |   N   |  DNE  |
| (Out) |       | (Out) |
+-------+-------+-------+
|   W   |CENTER |   E   |
|       |(Self) |       |
+-------+-------+-------+
|  DSW  |   S   |  DSE  |
| (Out) |       | (Out) |
+-------+-------+-------+
```

*Note: Inner corners (`cnw`, etc.) are logic subsets usually handled by specific tiling rules where `N` and `W` meet, etc.*

## 4. Example Definition

```xml
<border id="2" group="1">
    <!-- North Edge -->
    <borderitem edge="n" item="4531"/>
    <!-- East Edge -->
    <borderitem edge="e" item="4532"/>
    <!-- Outer Corner (Diagonal) -->
    <borderitem edge="dne" item="4540"/>
    <!-- Inner Corner -->
    <borderitem edge="cne" item="4536"/>
    <!-- ... others ... -->
</border>
```
