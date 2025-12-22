# Border Editor - Development Learnings

This document captures patterns, insights, and best practices discovered during the development of the Auto Border Editor improvements.

## Architecture Insights

### 1. Tileset Loading
- **g_materials.tilesets** is populated by the engine at startup - no need to parse XML manually
- `tilesets.xml` only contains `<include>` tags - actual tileset data is in included files
- Always use the pre-loaded data instead of re-parsing

### 2. Multi-Tab UI Pattern
When a dialog has multiple tabs (Border, Ground, Wall), each may need:
- Separate data storage (e.g., `m_borderEnabledTilesets`, `m_groundEnabledTilesets`)
- Tab-aware logic using `m_activeTab` to determine which widget to update
- Independent persistence per mode

### 3. wxWidgets Event Gotchas
| Widget | Event | Behavior |
|--------|-------|----------|
| `wxListBox` | `wxEVT_LISTBOX` | Standard selection |
| `wxComboBox` | `wxEVT_COMBOBOX` | Selection change |
| `wxCheckListBox` | `wxEVT_LISTBOX` | Only fires on label click, not checkbox |

**Solution for CheckListBox**: Bind `wxEVT_LISTBOX` to toggle checkbox:
```cpp
m_tilesetList->Bind(wxEVT_LISTBOX, [this](wxCommandEvent& ev) {
    int sel = m_tilesetList->GetSelection();
    if (sel != wxNOT_FOUND) {
        m_tilesetList->Check(sel, !m_tilesetList->IsChecked(sel));
    }
});
```

## Real-Time Persistence Pattern

Used for saving filter configuration immediately (like XML brush saves):

```cpp
void SaveFilterConfig() {
    wxString configDir = wxStandardPaths::Get().GetUserConfigDir() + 
                         wxFileName::GetPathSeparator() + ".rme";
    if (!wxDirExists(configDir)) wxMkdir(configDir);
    
    wxString configFile = configDir + wxFileName::GetPathSeparator() + "config.json";
    
    wxFile file(configFile, wxFile::write);
    if (file.IsOpened()) {
        file.Write(json);
        file.Close();
    }
}
```

**Config location**: `~/.rme/border_editor_filters.json`

## Common Pitfalls

1. **"Works in one tab, not the other"**
   - Check if `LoadTilesets()` or similar updates ALL relevant widgets
   - Use `m_activeTab` to determine which widget to update

2. **Blacklist vs Whitelist**
   - Whitelist (enabled) is more intuitive for filters
   - Empty whitelist = show all (first-time users)
   - Non-empty whitelist = show only selected

3. **Multiple ComboBoxes**
   - Border tab: `m_rawCategoryCombo`
   - Ground tab: `m_groundTilesetCombo`
   - Each needs separate update logic

## Files Modified

- `border_editor_window.h` - TilesetFilterDialog class, whitelists, persistence methods
- `border_editor_window.cpp` - Filter dialog implementation, LoadTilesets fixes
- Config saved to: `~/.rme/border_editor_filters.json`
