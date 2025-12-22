# Border Editor - Development Learnings

This document captures patterns, insights, and best practices discovered during the development of the Auto Border Editor improvements.

## Architecture Insights

### 1. Tileset Loading
- **g_materials.tilesets** is populated by the engine at startup - no need to parse XML manually
- `tilesets.xml` only contains `<include>` tags - actual tileset data is in included files
- Always use the pre-loaded data instead of re-parsing
- **Categories**: Assets may be in `TILESET_RAW`, `TILESET_TERRAIN`, or `TILESET_DOODAD`. Always check multiple categories if assets are missing (e.g., Mountains were in Terrain, not Raw).

### 2. Multi-Tab UI Pattern
When a dialog has multiple tabs (Border, Ground, Wall), each may need:
- Separate data storage (e.g., `m_borderEnabledTilesets`, `m_groundEnabledTilesets`)
- Tab-aware logic using `m_activeTab` to determine which widget to update
- **Critical Initialization**: Ensure data is populated for *every* tab. If logic only runs at startup, inactive tabs will remain empty/stale until manually refreshed.

### 3. wxWidgets Event Gotchas
| Widget | Event | Behavior |
|--------|-------|----------|
| `wxListBox` | `wxEVT_LISTBOX` | Standard selection |
| `wxComboBox` | `wxEVT_COMBOBOX` | Selection change |
| `wxCheckListBox` | `wxEVT_LISTBOX` | Only fires on label click, not checkbox |
| `wxNotebook` | `ChangeSelection` | **DOES NOT** fire `wxEVT_NOTEBOOK_PAGE_CHANGED` |
| `wxNotebook` | `SetSelection` | Fires `wxEVT_NOTEBOOK_PAGE_CHANGED` |

**Critical Trap**: If you use `ChangeSelection` (e.g., in custom toggle buttons), you MUST manually trigger any update logic (like `LoadTilesets()`) that would normally happen in `OnPageChanged`.

## Real-Time Persistence Pattern

Used for saving filter configuration immediately (like XML brush saves):
Config location: `~/.rme/border_editor_filters.json`

## Common Pitfalls

1. **"Works in one tab, not the other"**
   - Check if `LoadTilesets()` or similar updates ALL relevant widgets
   - Verify if the initialization logic runs for *inactive* tabs at startup

2. **Filter Logic (Whitelist)**
   - **Empty Whitelist**: Should mean "User selected nothing". Do not fallback to "Show All" or you risk auto-selecting the first item (e.g., "Addon...") when the user wants an empty state.
   - **Visual Feedback**: If filter is empty, ensure the UI (Palette/ComboBox) is explicitly cleared.

3. **Data Staleness on Tab Switch**
   - If using custom navigation (buttons), ensure data refresh (`LoadTilesets`) is called explicitly.
   - Do not rely on `OnPageChanged` if using `ChangeSelection`.

## Files Modified

- `border_editor_window.h` - TilesetFilterDialog class, whitelists, persistence methods
- `border_editor_window.cpp` - Filter dialog implementation, LoadTilesets fixes, OnModeSwitch fixes
- Config saved to: `~/.rme/border_editor_filters.json`
