//////////////////////////////////////////////////////////////////////
// Dear ImGui Panels - Header
// Modern UI panels for RME using Dear ImGui
//////////////////////////////////////////////////////////////////////

#ifndef RME_IMGUI_PANELS_H
#define RME_IMGUI_PANELS_H

#include <imgui.h>
#include <string>
#include <vector>

// Forward declarations
class Tile;
class Item;
class Editor;

namespace ImGuiPanels {

// Initialize the panels system
void Init();

// Shutdown the panels system
void Shutdown();

// Draw the Welcome Screen (startup dialog)
// Returns true if an action was taken (map opened, etc)
bool DrawWelcomeScreen(const std::vector<std::string>& recentFiles);
bool IsWelcomeScreenVisible();
void ShowWelcomeScreen();
void HideWelcomeScreen();

// Draw the performance monitor (FPS, graphics stats, input latency)
void DrawDebugOverlay(Editor* editor, int mouseX, int mouseY, Tile* hoverTile);

// Draw the tools panel (brush selection, zoom, floor)
void DrawToolsPanel(Editor* editor, int currentFloor, double currentZoom);

// Draw the main menu bar (File, Edit, View, Floor menus)
void DrawMainMenuBar(Editor* editor);

// Update RME-specific metrics (call from render loop)
void UpdateRMEMetrics(int textureBinds, int tilesRendered, int visibleTiles);

// Toggle panels visibility
void ToggleDebugOverlay();
void ToggleToolsPanel();

// Check visibility
bool IsDebugOverlayVisible();
bool IsToolsPanelVisible();

// Set overlay opacity (0.0 - 1.0)
void SetOverlayOpacity(float opacity);

} // namespace ImGuiPanels

#endif // RME_IMGUI_PANELS_H
