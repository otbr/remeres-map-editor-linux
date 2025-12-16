//////////////////////////////////////////////////////////////////////
// Dear ImGui Panels - Implementation
// Modern UI panels for RME using Dear ImGui
//////////////////////////////////////////////////////////////////////

#include "imgui_panels.h"
#include "editor.h"
#include "tile.h"
#include "item.h"
#include "position.h"
#include "map.h"

#include <imgui.h>

namespace ImGuiPanels {

// State
static bool g_ShowDebugOverlay = true;
static bool g_ShowToolsPanel = true;
static float g_OverlayOpacity = 0.85f;

void Init() {
    g_ShowDebugOverlay = true;
    g_ShowToolsPanel = true;
    g_OverlayOpacity = 0.85f;
}

void Shutdown() {
    // Nothing to clean up for now
}

void DrawDebugOverlay(Editor* editor, int mouseX, int mouseY, Tile* hoverTile) {
    if (!g_ShowDebugOverlay) {
        return;
    }
    
    // Set up overlay window flags (minimal decoration)
    ImGuiWindowFlags overlayFlags = 
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove;
    
    // Position in top-left corner with padding
    const float PAD = 10.0f;
    ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
    ImGui::SetNextWindowPos(ImVec2(workPos.x + PAD, workPos.y + PAD), ImGuiCond_Always);
    
    // Set window background opacity
    ImGui::SetNextWindowBgAlpha(g_OverlayOpacity);
    
    if (ImGui::Begin("##DebugOverlay", nullptr, overlayFlags)) {
        // FPS Counter
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.6f, 1.0f), "RME ImGui");
        ImGui::Separator();
        
        // Frame rate
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        
        ImGui::Separator();
        
        // Mouse position
        ImGui::Text("Mouse: %d, %d", mouseX, mouseY);
        
        // Tile info if hovering
        if (hoverTile) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.5f, 1.0f), "Tile");
            
            Position pos = hoverTile->getPosition();
            ImGui::Text("Pos: %d, %d, %d", pos.x, pos.y, pos.z);
            ImGui::Text("Items: %zu", hoverTile->items.size());
            
            if (hoverTile->ground) {
                ImGui::Text("Ground: %d", hoverTile->ground->getID());
            }
            
            // Use direct tile state checks
            if (hoverTile->isBlocking()) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "[Blocking]");
            }
            if (hoverTile->isPZ()) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[PZ]");
            }
        }
    }
    ImGui::End();
}

void DrawToolsPanel(Editor* editor, int currentFloor, double currentZoom) {
    if (!g_ShowToolsPanel) {
        return;
    }
    
    // Position in top-right corner
    ImGuiWindowFlags toolsFlags = 
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize;
    
    ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
    ImVec2 workSize = ImGui::GetMainViewport()->WorkSize;
    const float PAD = 10.0f;
    
    // Use Always to ensure panel appears in correct position (fixes invisible panel issue)
    ImGui::SetNextWindowPos(ImVec2(workPos.x + workSize.x - 180 - PAD, workPos.y + PAD), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(g_OverlayOpacity);
    
    if (ImGui::Begin("Tools", &g_ShowToolsPanel, toolsFlags)) {
        // Floor indicator
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 0.7f, 1.0f), "Navigation");
        ImGui::Separator();
        
        ImGui::Text("Floor: %d", currentFloor);
        ImGui::Text("Zoom: %.0f%%", (1.0 / currentZoom) * 100.0);
        
        ImGui::Spacing();
        
        // Quick actions section
        ImGui::TextColored(ImVec4(0.85f, 0.7f, 0.7f, 1.0f), "Quick Actions");
        ImGui::Separator();
        
        if (ImGui::Button("Toggle Debug", ImVec2(-1, 0))) {
            ToggleDebugOverlay();
        }
        
        ImGui::Spacing();
        
        // View settings
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.85f, 1.0f), "View Settings");
        ImGui::Separator();
        
        // Use global opacity directly (fixes desync issue)
        if (ImGui::SliderFloat("Opacity", &g_OverlayOpacity, 0.3f, 1.0f)) {
            // g_OverlayOpacity is modified directly by the slider
        }
    }
    ImGui::End();
}

void ToggleDebugOverlay() {
    g_ShowDebugOverlay = !g_ShowDebugOverlay;
}

void ToggleToolsPanel() {
    g_ShowToolsPanel = !g_ShowToolsPanel;
}

bool IsDebugOverlayVisible() {
    return g_ShowDebugOverlay;
}

bool IsToolsPanelVisible() {
    return g_ShowToolsPanel;
}

void SetOverlayOpacity(float opacity) {
    g_OverlayOpacity = (opacity < 0.0f) ? 0.0f : (opacity > 1.0f) ? 1.0f : opacity;
}

} // namespace ImGuiPanels
