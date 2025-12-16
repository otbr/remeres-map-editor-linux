//////////////////////////////////////////////////////////////////////
// Dear ImGui Panels - Header
// Modern UI panels for RME using Dear ImGui
//////////////////////////////////////////////////////////////////////

#ifndef RME_IMGUI_PANELS_H
#define RME_IMGUI_PANELS_H

#include <imgui.h>
#include <string>

// Forward declarations
class Tile;
class Item;
class Editor;

namespace ImGuiPanels {

// Initialize the panels system
void Init();

// Shutdown the panels system
void Shutdown();

// Draw the debug overlay (FPS, mouse position, tile info)
// This is always drawn in the top-left corner
void DrawDebugOverlay(Editor* editor, int mouseX, int mouseY, Tile* hoverTile);

// Draw the brush palette panel (prototype)
// void DrawBrushPalette();

// Draw item properties panel (prototype)
// void DrawItemProperties(Item* item);

// Toggle debug overlay visibility
void ToggleDebugOverlay();

// Check if debug overlay is visible
bool IsDebugOverlayVisible();

// Set overlay opacity (0.0 - 1.0)
void SetOverlayOpacity(float opacity);

} // namespace ImGuiPanels

#endif // RME_IMGUI_PANELS_H
