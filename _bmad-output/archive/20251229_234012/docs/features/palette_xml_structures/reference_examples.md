# Reference Examples: Palette XML Structures

## Tileset Definition (`data/materials/tilesets/grounds_nature.xml`)

Tilesets group brushes and raw items into categories for the editor UI.

```xml
<materials>
	<tileset name="Grounds - Nature">
		<terrain>
			<brush name="grass"/>
			<brush name="dirt"/>
			<!-- ... -->
		</terrain>
		<raw>
			<item id="372"/>
			<item fromid="4654" toid="4655"/>
			<!-- ... -->
		</raw>
	</tileset>
</materials>
```

## Ground Brush Definition (`data/materials/brushs/grounds.xml`)

Defines a specific ground brush, referenced by name in the tileset.

```xml
<brush name="grass" type="ground" lookid="4515" z-order="3200">
    <!-- Main tiles for the ground -->
    <item id="4515" chance="2500"/>
    <item id="4516" chance="10"/>

    <!-- Border definitions (referencing borders.xml IDs) -->
    <border align="outer" id="2"/>
    <border align="inner" to="none" id="1"/>
    
    <!-- Optional borders -->
    <optional id="120"/>
</brush>
```

## Border Definition (`data/materials/borders/borders.xml`)

Defines the auto-border logic for a specific ID.

```xml
<border id="2" group="1"> -- grass border --
    <borderitem edge="n" item="4531" />
    <borderitem edge="e" item="4532" />
    <borderitem edge="s" item="4533" />
    <borderitem edge="w" item="4534" />
    <borderitem edge="cnw" item="4535" />
    <borderitem edge="cne" item="4536" />
    <borderitem edge="csw" item="4537" />
    <borderitem edge="cse" item="4538" />
    <borderitem edge="dnw" item="4539" />
    <borderitem edge="dne" item="4540" />
    <borderitem edge="dsw" item="4541" />
    <borderitem edge="dse" item="4542" />
</border>
```

## Wall Brush Definition (`data/materials/brushs/walls.xml`)

Defines wall structures.

```xml
<brush name="stone wall" type="wall" server_lookid="1295">
    <wall type="horizontal">
        <item id="1295" chance="500"/>
        <door id="6251" type="normal" open="false"/>
    </wall>
    <wall type="vertical">
        <item id="1294" chance="400"/>
        <door id="6248" type="normal" open="false"/>
    </wall>
    <wall type="corner">
        <item id="1298" chance="1000"/>
    </wall>
    <wall type="pole">
        <item id="1296" chance="1000"/>
    </wall>
</brush>
```

## Complex Logic (Specifics)

Used in Ground brushes for specific neighbor interactions (e.g., Snow vs Sea).

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

## Relationships

1.  **Tileset** (`box`) contains **Brush Names** (`grass`).
2.  **Brush Declaration** (`grass`) defines items and refers to **Border IDs** (`2`).
3.  **Border Declaration** (`2`) defines the physical tiles for edges.
