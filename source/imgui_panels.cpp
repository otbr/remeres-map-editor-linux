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
#include "gui.h"
#include "settings.h"
#include "brush.h"

#include <imgui.h>
#include <chrono>
#include <algorithm>
#include <cmath>

namespace ImGuiPanels {

// Forward declarations for GUI callbacks
static void DoNewMap();
static void DoOpenMap();
static void DoSaveMap();
static void DoUndo();
static void DoRedo();

// === Panel State ===
static bool g_ShowToolsPanel = true;
static bool g_ShowPerformancePanel = false;  // Hidden by default, enabled via checkbox
static float g_OverlayOpacity = 0.85f;

// === Performance Metrics ===

static constexpr int HISTORY_SIZE = 120;

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

// Mouse tracking
static int g_LastMouseX = 0;
static int g_LastMouseY = 0;
static int g_MouseMoveCount = 0;
static float g_MouseLatency = 0.0f;
static float g_AvgMouseLatency = 0.0f;
static float g_TotalMouseLatency = 0.0f;
static std::chrono::steady_clock::time_point g_LastMouseMoveTime;

void Init() {
    g_ShowToolsPanel = true;
    g_ShowPerformancePanel = false;
    g_OverlayOpacity = 0.85f;
    g_HistoryIndex = 0;
    g_MinFps = 999.0f;
    g_MaxFps = 0.0f;
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

void Shutdown() {}

// === Welcome Screen State ===
static bool g_ShowWelcomeScreen = true;
static int g_SelectedRecentFile = -1;

bool DrawWelcomeScreen(const std::vector<std::string>& recentFiles) {
    if (!g_ShowWelcomeScreen) {
        return false;
    }
    
    bool actionTaken = false;
    
    // Get viewport size for fullscreen centered window
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;
    
    // Window size - 80% of viewport
    float windowWidth = viewportSize.x * 0.8f;
    float windowHeight = viewportSize.y * 0.8f;
    
    // Center position
    ImVec2 windowPos(
        viewportPos.x + (viewportSize.x - windowWidth) * 0.5f,
        viewportPos.y + (viewportSize.y - windowHeight) * 0.5f
    );
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.98f);
    
    ImGuiWindowFlags welcomeFlags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    
    if (ImGui::Begin("##WelcomeScreen", nullptr, welcomeFlags)) {
        float leftColumnWidth = windowWidth * 0.55f;
        float rightColumnWidth = windowWidth * 0.45f;
        
        // Left Column - Title and Buttons
        ImGui::BeginChild("##LeftColumn", ImVec2(leftColumnWidth, windowHeight), false);
        {
            ImGui::Dummy(ImVec2(0, windowHeight * 0.15f));
            
            // Title - Centered
            ImGui::PushFont(nullptr);  // Default font
            const char* title = "Canary's Map Editor";
            float titleWidth = ImGui::CalcTextSize(title).x;
            ImGui::SetCursorPosX((leftColumnWidth - titleWidth) * 0.5f);
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", title);
            
            // Version
            const char* version = "Version 4.0.0";
            float versionWidth = ImGui::CalcTextSize(version).x;
            ImGui::SetCursorPosX((leftColumnWidth - versionWidth) * 0.5f);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", version);
            ImGui::PopFont();
            
            ImGui::Dummy(ImVec2(0, windowHeight * 0.1f));
            
            // Buttons - Centered, monochromatic dark theme
            float buttonWidth = 180.0f;
            float buttonHeight = 40.0f;
            float buttonX = (leftColumnWidth - buttonWidth) * 0.5f;
            
            // Primary button - Open Map (Blue accent)
            ImGui::SetCursorPosX(buttonX);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.47f, 0.71f, 1.0f));  // Blue
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.55f, 0.80f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.40f, 0.60f, 1.0f));
            if (ImGui::Button("Open Map", ImVec2(buttonWidth, buttonHeight))) {
                g_ShowWelcomeScreen = false;
                g_gui.OpenMap();
                actionTaken = true;
            }
            ImGui::PopStyleColor(3);
            
            ImGui::Dummy(ImVec2(0, 8));
            
            // Secondary buttons - Dark gray
            ImGui::SetCursorPosX(buttonX);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
            if (ImGui::Button("New Map", ImVec2(buttonWidth, buttonHeight))) {
                g_ShowWelcomeScreen = false;
                g_gui.NewMap();
                actionTaken = true;
            }
            ImGui::PopStyleColor(3);
            
            ImGui::Dummy(ImVec2(0, 8));
            
            ImGui::SetCursorPosX(buttonX);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
            if (ImGui::Button("Preferences", ImVec2(buttonWidth, buttonHeight))) {
                // TODO: Open preferences
            }
            ImGui::PopStyleColor(3);
            
            // Bottom - Checkbox and credits
            ImGui::SetCursorPosY(windowHeight - 60);
            ImGui::SetCursorPosX(20);
            static bool showOnStartup = true;
            ImGui::Checkbox("Show this on startup", &showOnStartup);
            
            ImGui::SetCursorPosX(20);
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Linux by Habdel-Edenfield");
        }
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // Right Column - Recent Files
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
        ImGui::BeginChild("##RightColumn", ImVec2(rightColumnWidth, windowHeight), false);
        {
            ImGui::Dummy(ImVec2(0, 20));
            ImGui::SetCursorPosX(20);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Recent Maps");
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0, 10));
            
            // List recent files
            for (size_t i = 0; i < recentFiles.size(); ++i) {
                ImGui::SetCursorPosX(10);
                
                // Get just filename
                std::string fullPath = recentFiles[i];
                std::string filename = fullPath;
                size_t lastSlash = fullPath.find_last_of("/\\");
                if (lastSlash != std::string::npos) {
                    filename = fullPath.substr(lastSlash + 1);
                }
                
                // Selectable item
                bool isSelected = (g_SelectedRecentFile == (int)i);
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.3f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.35f, 0.45f, 1.0f));
                
                if (ImGui::Selectable(("##recent" + std::to_string(i)).c_str(), isSelected, 
                                      ImGuiSelectableFlags_AllowDoubleClick, 
                                      ImVec2(rightColumnWidth - 30, 50))) {
                    g_SelectedRecentFile = (int)i;
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        g_ShowWelcomeScreen = false;
                        g_gui.LoadMap(wxString(fullPath));
                        actionTaken = true;
                    }
                }
                
                // Draw text over selectable
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 48);
                ImGui::SetCursorPosX(20);
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", filename.c_str());
                ImGui::SetCursorPosX(20);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", fullPath.c_str());
                ImGui::Dummy(ImVec2(0, 6));
                
                ImGui::PopStyleColor(2);
            }
            
            if (recentFiles.empty()) {
                ImGui::SetCursorPosX(20);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No recent maps");
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
    ImGui::End();
    
    ImGui::PopStyleVar(2);
    
    return actionTaken;
}

bool IsWelcomeScreenVisible() {
    return g_ShowWelcomeScreen;
}

void ShowWelcomeScreen() {
    g_ShowWelcomeScreen = true;
}

void HideWelcomeScreen() {
    g_ShowWelcomeScreen = false;
}

void UpdateRMEMetrics(int textureBinds, int tilesRendered, int visibleTiles) {
    g_TextureBinds = textureBinds;
    g_TextureBindsHistory[g_HistoryIndex] = textureBinds;
    
    if (textureBinds > 0) {
        g_MinBinds = std::min(g_MinBinds, textureBinds);
        g_MaxBinds = std::max(g_MaxBinds, textureBinds);
        g_TotalBinds += textureBinds;
    }
}

void DrawDebugOverlay(Editor* editor, int mouseX, int mouseY, Tile* hoverTile) {
    // Update metrics even if not showing
    float currentFps = ImGui::GetIO().Framerate;
    float currentFrameTime = 1000.0f / (currentFps > 0 ? currentFps : 1.0f);
    
    g_FpsHistory[g_HistoryIndex] = currentFps;
    g_FrameTimeHistory[g_HistoryIndex] = currentFrameTime;
    g_HistoryIndex = (g_HistoryIndex + 1) % HISTORY_SIZE;
    
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

    // Don't draw if not visible
    if (!g_ShowPerformancePanel) {
        return;
    }
    
    // === Window Setup - MOVABLE performance panel ===
    ImGuiWindowFlags perfFlags = 
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings;
    
    // Initial position in top-left, but user can move it
    const float PAD = 10.0f;
    ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
    ImGui::SetNextWindowPos(ImVec2(workPos.x + PAD, workPos.y + PAD), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(g_OverlayOpacity);
    
    if (ImGui::Begin("RME Performance", &g_ShowPerformancePanel, perfFlags)) {
        // === FPS Section ===
        ImVec4 fpsColor = (currentFps >= 55) ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) :
                          (currentFps >= 30) ? ImVec4(1.0f, 1.0f, 0.4f, 1.0f) :
                                               ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        
        ImGui::TextColored(fpsColor, "FPS: %.0f", currentFps);
        ImGui::SameLine();
        ImGui::Text("(%.2f ms)", currentFrameTime);
        
        char overlay[32];
        snprintf(overlay, sizeof(overlay), "%.0f fps", currentFps);
        ImGui::PlotLines("##FPS", g_FpsHistory, HISTORY_SIZE, g_HistoryIndex, 
                         overlay, 0.0f, 120.0f, ImVec2(180, 35));
        
        ImGui::Text("Min:%.0f Max:%.0f Avg:%.0f", g_MinFps, g_MaxFps, g_AvgFps);
        
        // === Graphics Section ===
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.4f, 1.0f), "Graphics");
        
        ImVec4 bindsColor = (g_TextureBinds < 100) ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) :
                            (g_TextureBinds < 300) ? ImVec4(1.0f, 1.0f, 0.4f, 1.0f) :
                                                     ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        ImGui::TextColored(bindsColor, "Tex Binds: %d", g_TextureBinds);
        
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
        ImGui::Text("Avg: %.1f ms (%d)", g_AvgMouseLatency, g_MouseMoveCount);
        
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
        ImGuiWindowFlags_AlwaysAutoResize;
    
    // Get viewport info for proper positioning
    ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;
    ImVec2 workSize = ImGui::GetMainViewport()->WorkSize;
    
    // Position in top-right corner, flush against edge
    const float PAD = 0.0f;  // No padding - flush to corner
    const float PANEL_WIDTH = 140.0f;  // Compact width
    float posX = workPos.x + workSize.x - PANEL_WIDTH;
    float posY = workPos.y;
    
    ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(g_OverlayOpacity);
    
    if (ImGui::Begin("Tools", &g_ShowToolsPanel, toolsFlags)) {
        // Navigation section
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 0.7f, 1.0f), "Navigation");
        ImGui::Separator();
        
        ImGui::Text("Floor: %d", currentFloor);
        ImGui::Text("Zoom: %.0f%%", (1.0 / currentZoom) * 100.0);
        
        ImGui::Spacing();
        
        // Tools section
        ImGui::TextColored(ImVec4(0.85f, 0.7f, 0.7f, 1.0f), "Panels");
        ImGui::Separator();
        
        // Checkbox to toggle Performance Monitor
        ImGui::Checkbox("Performance", &g_ShowPerformancePanel);
        
        ImGui::Spacing();
        
        // View settings
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.85f, 1.0f), "View");
        ImGui::Separator();
        
        ImGui::SliderFloat("Opacity", &g_OverlayOpacity, 0.3f, 1.0f);
    }
    ImGui::End();
}

void ToggleDebugOverlay() {
    g_ShowPerformancePanel = !g_ShowPerformancePanel;
}

void ToggleToolsPanel() {
    g_ShowToolsPanel = !g_ShowToolsPanel;
}

bool IsDebugOverlayVisible() {
    return g_ShowPerformancePanel;
}

bool IsToolsPanelVisible() {
    return g_ShowToolsPanel;
}

void SetOverlayOpacity(float opacity) {
    g_OverlayOpacity = (opacity < 0.0f) ? 0.0f : (opacity > 1.0f) ? 1.0f : opacity;
}

// === GUI Callbacks ===
static void DoNewMap() { g_gui.NewMap(); }
static void DoOpenMap() { g_gui.OpenMap(); }
static void DoSaveMap() { g_gui.SaveMap(); }
static void DoUndo() { g_gui.DoUndo(); }
static void DoRedo() { g_gui.DoRedo(); }

// === ImGui Main Menu Bar ===
void DrawMainMenuBar(Editor* editor) {
    if (ImGui::BeginMainMenuBar()) {
        // File Menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) { DoNewMap(); }
            if (ImGui::MenuItem("Open", "Ctrl+O")) { DoOpenMap(); }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "Ctrl+S", false, editor != nullptr)) { DoSaveMap(); }
            if (ImGui::MenuItem("Save As..", "Ctrl+Shift+S", false, editor != nullptr)) { g_gui.SaveMapAs(); }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Ctrl+Q")) { g_gui.root->Close(); }
            ImGui::EndMenu();
        }
        
        // Edit Menu
        if (ImGui::BeginMenu("Edit")) {
            bool canUndo = editor && editor->canUndo();
            bool canRedo = editor && editor->canRedo();
            bool canPaste = editor && editor->copybuffer.canPaste();
            
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, canUndo)) { DoUndo(); }
            if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, canRedo)) { DoRedo(); }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X", false, editor != nullptr)) { g_gui.DoCut(); }
            if (ImGui::MenuItem("Copy", "Ctrl+C", false, editor != nullptr)) { g_gui.DoCopy(); }
            if (ImGui::MenuItem("Paste", "Ctrl+V", false, canPaste)) { g_gui.PreparePaste(); }
            ImGui::EndMenu();
        }
        
        // View Menu
        if (ImGui::BeginMenu("View")) {
            bool showGrid = g_settings.getBoolean(Config::SHOW_GRID);
            bool showShade = g_settings.getBoolean(Config::SHOW_SHADE);
            bool showHouses = g_settings.getBoolean(Config::SHOW_HOUSES);
            bool showMonsters = g_settings.getBoolean(Config::SHOW_MONSTERS);
            bool showNpcs = g_settings.getBoolean(Config::SHOW_NPCS);
            
            if (ImGui::MenuItem("Grid", "Shift+G", &showGrid)) {
                g_settings.setInteger(Config::SHOW_GRID, showGrid);
                g_gui.RefreshView();
            }
            if (ImGui::MenuItem("Shade", "Q", &showShade)) {
                g_settings.setInteger(Config::SHOW_SHADE, showShade);
                g_gui.RefreshView();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Houses", "Ctrl+H", &showHouses)) {
                g_settings.setInteger(Config::SHOW_HOUSES, showHouses);
                g_gui.RefreshView();
            }
            if (ImGui::MenuItem("Monsters", "F", &showMonsters)) {
                g_settings.setInteger(Config::SHOW_MONSTERS, showMonsters);
                g_gui.RefreshView();
            }
            if (ImGui::MenuItem("NPCs", "X", &showNpcs)) {
                g_settings.setInteger(Config::SHOW_NPCS, showNpcs);
                g_gui.RefreshView();
            }
            ImGui::Separator();
            ImGui::MenuItem("Performance", nullptr, &g_ShowPerformancePanel);
            ImGui::MenuItem("Tools Panel", nullptr, &g_ShowToolsPanel);
            ImGui::EndMenu();
        }
        
        // Floor Menu
        if (ImGui::BeginMenu("Floor")) {
            int currentFloor = g_gui.GetCurrentFloor();
            for (int f = 0; f <= 15; ++f) {
                char label[16];
                snprintf(label, sizeof(label), "Floor %d", f);
                if (ImGui::MenuItem(label, nullptr, f == currentFloor, editor != nullptr)) {
                    g_gui.ChangeFloor(f);
                }
            }
            ImGui::EndMenu();
        }
        
        // Right-aligned status
        float rightPad = 150.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - rightPad);
        ImGui::TextDisabled("RME 4.0");
        
        ImGui::EndMainMenuBar();
    }
}

} // namespace ImGuiPanels

