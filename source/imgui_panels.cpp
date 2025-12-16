//////////////////////////////////////////////////////////////////////
// Dear ImGui Panels - Implementation
// Performance Monitor & Tools for RME using Dear ImGui
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
#include <cmath>

namespace ImGuiPanels {

// === Panel State ===
static bool g_ShowDebugOverlay = true;
static bool g_ShowToolsPanel = true;
static float g_OverlayOpacity = 0.85f;

// === Performance Metrics ===
static constexpr int HISTORY_SIZE = 120;  // 2 seconds at 60fps

// Frame timing
static float g_FpsHistory[HISTORY_SIZE] = {0};
static float g_FrameTimeHistory[HISTORY_SIZE] = {0};
static int g_HistoryIndex = 0;

// Statistics
static float g_MinFps = 999.0f;
static float g_MaxFps = 0.0f;
static float g_AvgFps = 0.0f;
static int g_FrameCount = 0;
static float g_TotalFrameTime = 0.0f;

// RME-specific metrics
static int g_TextureBinds = 0;
static int g_TextureBindsHistory[HISTORY_SIZE] = {0};
static int g_MinBinds = 99999;
static int g_MaxBinds = 0;
static int g_TotalBinds = 0;

static int g_TilesRendered = 0;
static int g_VisibleTiles = 0;

// Mouse tracking for input latency analysis
static int g_LastMouseX = 0;
static int g_LastMouseY = 0;
static int g_MouseMoveCount = 0;
static float g_MouseLatency = 0.0f;
static float g_AvgMouseLatency = 0.0f;
static float g_TotalMouseLatency = 0.0f;
static std::chrono::steady_clock::time_point g_LastMouseMoveTime;

// Render timing
static std::chrono::steady_clock::time_point g_FrameStartTime;
static float g_LastRenderTime = 0.0f;

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
    g_TotalMouseLatency = 0.0f;
    g_MinBinds = 99999;
    g_MaxBinds = 0;
    g_TotalBinds = 0;
    g_LastMouseMoveTime = std::chrono::steady_clock::now();
    
    for (int i = 0; i < HISTORY_SIZE; i++) {
        g_FpsHistory[i] = 0.0f;
        g_FrameTimeHistory[i] = 0.0f;
        g_TextureBindsHistory[i] = 0;
    }
}

void Shutdown() {
    // Nothing to clean up
}

void UpdateRMEMetrics(int textureBinds, int tilesRendered, int visibleTiles) {
    g_TextureBinds = textureBinds;
    g_TilesRendered = tilesRendered;
    g_VisibleTiles = visibleTiles;
    
    // Update binds history
    g_TextureBindsHistory[g_HistoryIndex] = textureBinds;
    
    // Update binds stats
    if (textureBinds > 0) {
        g_MinBinds = std::min(g_MinBinds, textureBinds);
        g_MaxBinds = std::max(g_MaxBinds, textureBinds);
        g_TotalBinds += textureBinds;
    }
}

void DrawDebugOverlay(Editor* editor, int mouseX, int mouseY, Tile* hoverTile) {
    if (!g_ShowDebugOverlay) {
        return;
    }
    
    // === Update Metrics ===
    float currentFps = ImGui::GetIO().Framerate;
    float currentFrameTime = 1000.0f / (currentFps > 0 ? currentFps : 1.0f);
    
    g_FpsHistory[g_HistoryIndex] = currentFps;
    g_FrameTimeHistory[g_HistoryIndex] = currentFrameTime;
    g_HistoryIndex = (g_HistoryIndex + 1) % HISTORY_SIZE;
    
    // Stats
    g_MinFps = std::min(g_MinFps, currentFps);
    g_MaxFps = std::max(g_MaxFps, currentFps);
    g_FrameCount++;
    g_TotalFrameTime += currentFrameTime;
    g_AvgFps = 1000.0f / (g_TotalFrameTime / g_FrameCount);
    
    // Mouse latency tracking
    if (mouseX != g_LastMouseX || mouseY != g_LastMouseY) {
        auto now = std::chrono::steady_clock::now();
        g_MouseLatency = std::chrono::duration<float, std::milli>(now - g_LastMouseMoveTime).count();
        g_LastMouseMoveTime = now;
        g_LastMouseX = mouseX;
        g_LastMouseY = mouseY;
        g_MouseMoveCount++;
        g_TotalMouseLatency += g_MouseLatency;
        g_AvgMouseLatency = g_TotalMouseLatency / g_MouseMoveCount;
    }
    
    // === Window Setup ===
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
    
    if (ImGui::Begin("##PerfMonitor", nullptr, overlayFlags)) {
        // === Header ===
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "RME Performance");
        ImGui::Separator();
        
        // === FPS Section ===
        ImVec4 fpsColor = (currentFps >= 55) ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) :
                          (currentFps >= 30) ? ImVec4(1.0f, 1.0f, 0.4f, 1.0f) :
                                               ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        
        ImGui::TextColored(fpsColor, "FPS: %.0f", currentFps);
        ImGui::SameLine();
        ImGui::Text("(%.2f ms)", currentFrameTime);
        
        // FPS Graph
        char overlay[32];
        snprintf(overlay, sizeof(overlay), "%.0f fps", currentFps);
        ImGui::PlotLines("##FPS", g_FpsHistory, HISTORY_SIZE, g_HistoryIndex, 
                         overlay, 0.0f, 120.0f, ImVec2(180, 35));
        
        ImGui::Text("Min:%.0f Max:%.0f Avg:%.0f", g_MinFps, g_MaxFps, g_AvgFps);
        
        // === Graphics Section ===
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.4f, 1.0f), "Graphics");
        
        // Texture binds with color coding
        ImVec4 bindsColor = (g_TextureBinds < 100) ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) :
                            (g_TextureBinds < 300) ? ImVec4(1.0f, 1.0f, 0.4f, 1.0f) :
                                                     ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        ImGui::TextColored(bindsColor, "Tex Binds: %d", g_TextureBinds);
        
        // Texture binds graph
        float bindsHistoryFloat[HISTORY_SIZE];
        for (int i = 0; i < HISTORY_SIZE; i++) {
            bindsHistoryFloat[i] = static_cast<float>(g_TextureBindsHistory[i]);
        }
        ImGui::PlotLines("##Binds", bindsHistoryFloat, HISTORY_SIZE, g_HistoryIndex,
                         nullptr, 0.0f, 500.0f, ImVec2(180, 25));
        
        if (g_FrameCount > 0) {
            ImGui::Text("Min:%d Max:%d Avg:%d", g_MinBinds, g_MaxBinds, 
                        g_TotalBinds / g_FrameCount);
        }
        
        // Tiles info
        ImGui::Text("Tiles: %d visible", g_VisibleTiles);
        
        // ImGui internal
        ImGui::Text("ImGui: %d vtx, %d idx", 
                    ImGui::GetIO().MetricsRenderVertices,
                    ImGui::GetIO().MetricsRenderIndices);
        
        // === Input Section ===
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.5f, 0.9f, 1.0f), "Input");
        ImGui::Text("Mouse: %d, %d", mouseX, mouseY);
        
        ImVec4 latencyColor = (g_MouseLatency < 20) ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) :
                              (g_MouseLatency < 50) ? ImVec4(1.0f, 1.0f, 0.4f, 1.0f) :
                                                      ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        ImGui::TextColored(latencyColor, "Latency: %.1f ms", g_MouseLatency);
        ImGui::Text("Avg: %.1f ms (%d moves)", g_AvgMouseLatency, g_MouseMoveCount);
        
        // === Tile Info ===
        if (hoverTile) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.5f, 1.0f), "Tile");
            
            Position pos = hoverTile->getPosition();
            ImGui::Text("Pos: %d,%d,%d", pos.x, pos.y, pos.z);
            ImGui::Text("Items: %zu", hoverTile->items.size());
            
            if (hoverTile->ground) {
                ImGui::Text("Ground: %d", hoverTile->ground->getID());
            }
            
            if (hoverTile->isBlocking()) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "[Blocking]");
            }
        }
        
        // === Reset Button ===
        ImGui::Separator();
        if (ImGui::Button("Reset Stats", ImVec2(-1, 0))) {
            g_MinFps = 999.0f;
            g_MaxFps = 0.0f;
            g_FrameCount = 0;
            g_TotalFrameTime = 0.0f;
            g_MouseMoveCount = 0;
            g_TotalMouseLatency = 0.0f;
            g_MinBinds = 99999;
            g_MaxBinds = 0;
            g_TotalBinds = 0;
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
        
        if (ImGui::Button("Toggle Monitor", ImVec2(-1, 0))) {
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
