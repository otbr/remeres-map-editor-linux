# Deep Dive Analysis: Palette System

## 1. Tileset Organization (`data/materials/tilesets/*.xml`)

Tilesets are purely organizational. They don't define the *behavior* of brushes, but rather which brushes appear in which palette category in the editor UI.

### Structure
```xml
<materials>
	<tileset name="Grounds - Nature">
		<terrain>
            <!-- References brushes defined in brushs/*.xml -->
			<brush name="grass"/>
			<brush name="dirt"/>
		</terrain>
		<raw>
            <!-- Direct item references (no auto-border logic) -->
			<item id="372"/>
			<item fromid="4654" toid="4655"/>
		</raw>
	</tileset>
</materials>
```

**Implication for Editor:**
- When creating a new Ground/Wall, we must also decide which **Tileset** it belongs to.
- The user should ideally be able to select an existing Tileset (e.g., "Grounds - Nature") or create a new one.
- We need to modify `tilesets.xml` or valid sub-files to include the new brush name.

## 2. Advanced Brush Structures

### Composite Tiles (Doodads/Grounds)
Seen in `doodads.xml` (like `snowy rocks`), composite tiles allow a single brush "click" to place a Multi-Tile structure.

```xml
<brush name="snowy rocks" type="doodad" ...>
    <composite chance="8">
        <tile x="0" y="0"> <item id="7016"/> </tile>
        <tile x="1" y="0"> <item id="7017"/> </tile>
    </composite>
</brush>
```
- **chance**: Probability of this specific variation being chosen.
- **tile**: Relative coordinates (x, y) and the item ID to place there.
- **Implication**: The editor needs a "Composite Editor" or "Multi-tile Editor" mode if we want to fully support creating these complex brushes. For V1, we might restrict to single-tile brushes or simple randomization.

### Friend Brushes
```xml
<brush name="grass open stone pile" ...>
    <friend name="grass"/>
</brush>
```
- **Function**: a "Start" brush connects seamlessly to a "Friend" brush.
- The "Friend" does NOT draw a border against the "Start" brush.
- **Example**: `grass` and `grass open stone pile`. They blend without a border.

### Z-Order
- **Attribute**: `z-order="3200"`
- **Function**: Determines drawing order when multiple borders/brushes overlap. Higher numbers result in the brush being drawn "on top".
- **Critical**: When creating a new ground, `z-order` is essential for correct layering (e.g., "Grass" on top of "Dirt").

## 3. Specific Border Rules (The "Edge Case" Logic)

The `<specific>` block in `grounds.xml` handles complex transition logic that standard 12-tile borders cannot.

**Scenario**: Snow bordering Sea.
Standard behavior: Snow draws its "inner" border.
Problem: If Snow touches Sea, we might want a specific "Snowy Sea" tile instead of the generic Snow border.

```xml
<specific>
    <conditions>
        <match_border id="6" edge="s"/> <!-- If South neighbor has border ID 6 (Ice?) -->
        <match_border id="5"  edge="e"/> <!-- And East neighbor has border ID 5 (Sea?) -->
    </conditions>
    <actions>
        <replace_border id="5" edge="e" with="6652"/> <!-- Replace the border item at East -->
    </actions>
</specific>
```

**Implication**:
- This is a rule-engine execution.
- Supporting this in a UI is **complex**.
- **Recommendation for V1**: Do not build a UI for `<specific>` rules. Allow users to edit the XML manually for these advanced cases, or provide a simple text editor snippet view. Focus the UI on the 95% use case: Standard 12-tile borders.

## 4. Doodads vs Carpets
- **Doodads**: Random objects placed *on top* of grounds. Can be blocking.
- **Carpets**: behave like borders but "overlay" existing ground without fully replacing logic. Defined with `type="carpet"` in `doodads.xml` (confusingly).
- **Structure**:
```xml
<brush name="green leaves" type="carpet" server_lookid="13766">
    <carpet align="n"   id="13755"/>
    <carpet align="center" id="13766"/>
    <!-- ... -->
</brush>
```
- This is effectively a "Border" definition *inline* within the brush, plus a center tile.
