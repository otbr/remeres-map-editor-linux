//////////////////////////////////////////////////////////////////////
// Dear ImGui wxWidgets Backend - Implementation
// Custom backend for integrating Dear ImGui with wxWidgets/OpenGL
//////////////////////////////////////////////////////////////////////

#include "imgui_impl_wx.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include <chrono>

namespace ImGuiWx {

// Internal state
static wxWindow* g_Window = nullptr;
static std::chrono::steady_clock::time_point g_LastTime;
static bool g_Initialized = false;

// Helper to convert wxWidgets key code to ImGui key
static ImGuiKey WxKeyToImGuiKey(int keyCode) {
    switch (keyCode) {
        case WXK_TAB: return ImGuiKey_Tab;
        case WXK_LEFT: return ImGuiKey_LeftArrow;
        case WXK_RIGHT: return ImGuiKey_RightArrow;
        case WXK_UP: return ImGuiKey_UpArrow;
        case WXK_DOWN: return ImGuiKey_DownArrow;
        case WXK_PAGEUP: return ImGuiKey_PageUp;
        case WXK_PAGEDOWN: return ImGuiKey_PageDown;
        case WXK_HOME: return ImGuiKey_Home;
        case WXK_END: return ImGuiKey_End;
        case WXK_INSERT: return ImGuiKey_Insert;
        case WXK_DELETE: return ImGuiKey_Delete;
        case WXK_BACK: return ImGuiKey_Backspace;
        case WXK_SPACE: return ImGuiKey_Space;
        case WXK_RETURN: return ImGuiKey_Enter;
        case WXK_ESCAPE: return ImGuiKey_Escape;
        case WXK_CONTROL: return ImGuiKey_LeftCtrl;
        case WXK_SHIFT: return ImGuiKey_LeftShift;
        case WXK_ALT: return ImGuiKey_LeftAlt;
        case 'A': return ImGuiKey_A;
        case 'C': return ImGuiKey_C;
        case 'V': return ImGuiKey_V;
        case 'X': return ImGuiKey_X;
        case 'Y': return ImGuiKey_Y;
        case 'Z': return ImGuiKey_Z;
        default: return ImGuiKey_None;
    }
}

bool Init(wxWindow* window) {
    if (g_Initialized) {
        return true;
    }
    
    g_Window = window;
    g_LastTime = std::chrono::steady_clock::now();
    
    ImGuiIO& io = ImGui::GetIO();
    
    // Setup backend capabilities
    io.BackendPlatformName = "imgui_impl_wx";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    
    // Setup display size
    wxSize size = window->GetClientSize();
    io.DisplaySize = ImVec2(static_cast<float>(size.x), static_cast<float>(size.y));
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    
    // Apply our dark theme
    ApplyDarkTheme();
    
    g_Initialized = true;
    return true;
}

void NewFrame() {
    if (!g_Initialized || !g_Window) {
        return;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    
    // Update display size
    wxSize size = g_Window->GetClientSize();
    io.DisplaySize = ImVec2(static_cast<float>(size.x), static_cast<float>(size.y));
    
    // Update delta time
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - g_LastTime).count();
    io.DeltaTime = deltaTime > 0.0f ? deltaTime : (1.0f / 60.0f);
    g_LastTime = currentTime;
    
    // Update mouse position
    if (g_Window->HasFocus()) {
        wxPoint mousePos = wxGetMousePosition();
        wxPoint windowPos = g_Window->ScreenToClient(mousePos);
        io.AddMousePosEvent(static_cast<float>(windowPos.x), static_cast<float>(windowPos.y));
    }
}

bool ProcessMouseEvent(wxMouseEvent& event) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Mouse position
    io.AddMousePosEvent(static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
    
    // Mouse buttons
    if (event.LeftDown() || event.LeftUp()) {
        io.AddMouseButtonEvent(0, event.LeftDown());
    }
    if (event.RightDown() || event.RightUp()) {
        io.AddMouseButtonEvent(1, event.RightDown());
    }
    if (event.MiddleDown() || event.MiddleUp()) {
        io.AddMouseButtonEvent(2, event.MiddleDown());
    }
    
    // Mouse wheel
    if (event.GetWheelRotation() != 0) {
        float wheel = static_cast<float>(event.GetWheelRotation()) / 
                      static_cast<float>(event.GetWheelDelta());
        if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
            io.AddMouseWheelEvent(wheel, 0.0f);
        } else {
            io.AddMouseWheelEvent(0.0f, wheel);
        }
    }
    
    return io.WantCaptureMouse;
}

bool ProcessKeyEvent(wxKeyEvent& event) {
    ImGuiIO& io = ImGui::GetIO();
    
    bool pressed = (event.GetEventType() == wxEVT_KEY_DOWN);
    ImGuiKey key = WxKeyToImGuiKey(event.GetKeyCode());
    
    if (key != ImGuiKey_None) {
        io.AddKeyEvent(key, pressed);
    }
    
    // Modifiers
    io.AddKeyEvent(ImGuiKey_ModCtrl, event.ControlDown());
    io.AddKeyEvent(ImGuiKey_ModShift, event.ShiftDown());
    io.AddKeyEvent(ImGuiKey_ModAlt, event.AltDown());
    
    return io.WantCaptureKeyboard;
}

void UpdateDisplaySize(wxSizeEvent& event) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(event.GetSize().x), 
                            static_cast<float>(event.GetSize().y));
}

void Shutdown() {
    g_Window = nullptr;
    g_Initialized = false;
}

bool WantCaptureMouse() {
    return ImGui::GetIO().WantCaptureMouse;
}

bool WantCaptureKeyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}

void ApplyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // VS Code inspired monocromatic dark theme - slim and elegant
    style.WindowRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    
    // Slim title bar (reduced padding for menu-like appearance)
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);  // Left-aligned title
    style.WindowPadding = ImVec2(6, 4);  // Slimmer padding
    style.FramePadding = ImVec2(4, 2);   // Slimmer frame
    style.ItemSpacing = ImVec2(6, 3);    // Compact spacing
    style.ItemInnerSpacing = ImVec2(4, 2);
    
    ImVec4* colors = style.Colors;
    
    // Background colors (dark gray)
    colors[ImGuiCol_WindowBg]           = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.12f, 0.12f, 0.12f, 0.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
    
    // Border
    colors[ImGuiCol_Border]             = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_BorderShadow]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Frame backgrounds
    colors[ImGuiCol_FrameBg]            = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    
    // Title bar
    colors[ImGuiCol_TitleBg]            = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.10f, 0.10f, 0.10f, 0.75f);
    
    // Menu bar
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
    colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    
    // Check mark
    colors[ImGuiCol_CheckMark]          = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    
    // Slider
    colors[ImGuiCol_SliderGrab]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]   = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
    
    // Button
    colors[ImGuiCol_Button]             = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    
    // Header (selectable, tree nodes)
    colors[ImGuiCol_Header]             = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    
    // Separator
    colors[ImGuiCol_Separator]          = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_SeparatorActive]    = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    
    // Resize grip
    colors[ImGuiCol_ResizeGrip]         = ImVec4(0.30f, 0.30f, 0.30f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.45f, 0.45f, 0.45f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.55f, 0.55f, 0.55f, 0.95f);
    
    // Tab
    colors[ImGuiCol_Tab]                = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    
    // Text
    colors[ImGuiCol_Text]               = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.30f, 0.30f, 0.50f, 0.35f);
    
    // Drag drop
    colors[ImGuiCol_DragDropTarget]     = ImVec4(0.50f, 0.50f, 0.50f, 0.90f);
    
    // Navigation
    colors[ImGuiCol_NavHighlight]       = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]  = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    
    // Modal
    colors[ImGuiCol_ModalWindowDimBg]   = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

} // namespace ImGuiWx
