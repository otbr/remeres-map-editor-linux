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
#include <chrono>
#include <algorithm>

namespace ImGuiPanels {

// State
static bool g_ShowDebugOverlay = true;
static bool g_ShowToolsPanel = true;
static float g_OverlayOpacity = 0.85f;

// Performance tracking
static constexpr int FPS_HISTORY_SIZE = 120;  // 2 seconds at 60fps
static float g_FpsHistory[FPS_HISTORY_SIZE] = {0};
static float g_FrameTimeHistory[FPS_HISTORY_SIZE] = {0};
static int g_HistoryIndex = 0;
static float g_MinFps = 999.0f;
static float g_MaxFps = 0.0f;
static float g_AvgFps = 0.0f;
static int g_FrameCount = 0;
static float g_TotalFrameTime = 0.0f;

// Mouse tracking for responsiveness analysis
static int g_LastMouseX = 0;
static int g_LastMouseY = 0;
static int g_MouseMoveCount = 0;
static float g_MouseLatency = 0.0f;
static std::chrono::steady_clock::time_point g_LastMouseMoveTime;

void Init() {
    g_ShowDebugOverlay = true;
    g_ShowToolsPanel = true;
    g_OverlayOpacity = 0.85f;
    g_HistoryIndex = 0;
    g_MinFps = 999.0f;
    g_MaxFps = 0.0f;
    g_AvgFps = 0.0f;
    g_FrameCount = 0;
    g_TotalFrameTime = 0.0f;
    g_MouseMoveCount = 0;
    g_LastMouseMoveTime = std::chrono::steady_clock::now();
    
    for (int i = 0; i < FPS_HISTORY_SIZE; i++) {
        g_FpsHistory[i] = 0.0f;
        g_FrameTimeHistory[i] = 0.0f;
    }
}

void Shutdown() {
    // Nothing to clean up
}

void DrawDebugOverlay(Editor* editor, int mouseX, int mouseY, Tile* hoverTile) {
    if (!g_ShowDebugOverlay) {
        return;
    }
    
    // Update performance history
    float currentFps = ImGui::GetIO().Framerate;
    float currentFrameTime = 1000.0f / (currentFps > 0 ? currentFps : 1.0f);
    
    g_FpsHistory[g_HistoryIndex] = currentFps;
    g_FrameTimeHistory[g_HistoryIndex] = currentFrameTime;
    g_HistoryIndex = (g_HistoryIndex + 1) % FPS_HISTORY_SIZE;
    
    // Update stats
    g_MinFps = std::min(g_MinFps, currentFps);
    g_MaxFps = std::max(g_MaxFps, currentFps);
    g_FrameCount++;
    g_TotalFrameTime += currentFrameTime;
    g_AvgFps = 1000.0f / (g_TotalFrameTime / g_FrameCount);
    
    // Track mouse movement for latency analysis
    if (mouseX != g_LastMouseX || mouseY != g_LastMouseY) {
        auto now = std::chrono::steady_clock::now();
        g_MouseLatency = std::chrono::duration<float, std::milli>(now - g_LastMouseMoveTime).count();
        g_LastMouseMoveTime = now;
        g_LastMouseX = mouseX;
        g_LastMouseY = mouseY;
        g_MouseMoveCount++;
    }
    
    // Window flags
    ImGuiWindowFlags overlayFlags = 
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove;
    
    const float PAD = 10.0f;
    ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
    ImGui::SetNextWindowPos(ImVec2(workPos.x + PAD, workPos.y + PAD), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(g_OverlayOpacity);
    
    if (ImGui::Begin("##DebugOverlay", nullptr, overlayFlags)) {
        // Header
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Performance Monitor");
        ImGui::Separator();
        
        // FPS with color coding
        ImVec4 fpsColor;
        if (currentFps >= 55) {
            fpsColor = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);  // Green
        } else if (currentFps >= 30) {
            fpsColor = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);  // Yellow
        } else {
            fpsColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);  // Red
        }
        
        ImGui::TextColored(fpsColor, "FPS: %.1f", currentFps);
        ImGui::Text("Frame: %.2f ms", currentFrameTime);
        
        // FPS Graph
        ImGui::PlotLines("##FpsGraph", g_FpsHistory, FPS_HISTORY_SIZE, g_HistoryIndex, 
                         nullptr, 0.0f, 120.0f, ImVec2(150, 40));
        
        // Stats
        ImGui::Text("Min: %.0f Max: %.0f Avg: %.0f", g_MinFps, g_MaxFps, g_AvgFps);
        
        ImGui::Separator();
        
        // Mouse tracking
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.9f, 1.0f), "Input Latency");
        ImGui::Text("Mouse: %d, %d", mouseX, mouseY);
        ImGui::Text("Delta: %.1f ms", g_MouseLatency);
        ImGui::Text("Moves: %d", g_MouseMoveCount);
        
        // Warning if latency is high
        if (g_MouseLatency > 50.0f) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "High latency!");
        }
        
        // Tile info
        if (hoverTile) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.5f, 1.0f), "Tile");
            
            Position pos = hoverTile->getPosition();
            ImGui::Text("Pos: %d,%d,%d", pos.x, pos.y, pos.z);
            ImGui::Text("Items: %zu", hoverTile->items.size());
            
            if (hoverTile->ground) {
                ImGui::Text("Ground: %d", hoverTile->ground->getID());
            }
        }
        
        // ImGui internal stats
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.9f, 1.0f), "ImGui Stats");
        ImGui::Text("Vertices: %d", ImGui::GetIO().MetricsRenderVertices);
        ImGui::Text("Indices: %d", ImGui::GetIO().MetricsRenderIndices);
        ImGui::Text("Windows: %d", ImGui::GetIO().MetricsRenderWindows);
        
        // Reset button
        if (ImGui::Button("Reset Stats")) {
            g_MinFps = 999.0f;
            g_MaxFps = 0.0f;
            g_FrameCount = 0;
            g_TotalFrameTime = 0.0f;
            g_MouseMoveCount = 0;
        }
    }
    ImGui::End();
}

void DrawToolsPanel(Editor* editor, int currentFloor, double currentZoom) {
    if (!g_ShowToolsPanel) {
        return;
    }
    
    ImGuiWindowFlags toolsFlags = 
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize;
    
    ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
    ImVec2 workSize = ImGui::GetMainViewport()->WorkSize;
    const float PAD = 10.0f;
    
    ImGui::SetNextWindowPos(ImVec2(workPos.x + workSize.x - 180 - PAD, workPos.y + PAD), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(g_OverlayOpacity);
    
    if (ImGui::Begin("Tools", &g_ShowToolsPanel, toolsFlags)) {
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 0.7f, 1.0f), "Navigation");
        ImGui::Separator();
        
        ImGui::Text("Floor: %d", currentFloor);
        ImGui::Text("Zoom: %.0f%%", (1.0 / currentZoom) * 100.0);
        
        ImGui::Spacing();
        
        ImGui::TextColored(ImVec4(0.85f, 0.7f, 0.7f, 1.0f), "Quick Actions");
        ImGui::Separator();
        
        if (ImGui::Button("Toggle Debug", ImVec2(-1, 0))) {
            ToggleDebugOverlay();
        }
        
        ImGui::Spacing();
        
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.85f, 1.0f), "View Settings");
        ImGui::Separator();
        
        ImGui::SliderFloat("Opacity", &g_OverlayOpacity, 0.3f, 1.0f);
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
