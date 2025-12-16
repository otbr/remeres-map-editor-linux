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

// Draw the tools panel (brush selection, zoom, floor)
void DrawToolsPanel(Editor* editor, int currentFloor, double currentZoom);

// Toggle debug overlay visibility
void ToggleDebugOverlay();

// Toggle tools panel visibility
void ToggleToolsPanel();

// Check if debug overlay is visible
bool IsDebugOverlayVisible();

// Check if tools panel is visible
bool IsToolsPanelVisible();

// Set overlay opacity (0.0 - 1.0)
void SetOverlayOpacity(float opacity);

} // namespace ImGuiPanels

#endif // RME_IMGUI_PANELS_H
