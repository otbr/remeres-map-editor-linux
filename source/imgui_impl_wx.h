//////////////////////////////////////////////////////////////////////
// Dear ImGui wxWidgets Backend - Header
// Custom backend for integrating Dear ImGui with wxWidgets/OpenGL
//////////////////////////////////////////////////////////////////////

#ifndef RME_IMGUI_IMPL_WX_H
#define RME_IMGUI_IMPL_WX_H

#include <imgui.h>

class wxWindow;
class wxMouseEvent;
class wxKeyEvent;
class wxSizeEvent;

namespace ImGuiWx {

// Initialize the wxWidgets backend
// Call after ImGui::CreateContext() and ImGui_ImplOpenGL3_Init()
bool Init(wxWindow* window);

// Start a new frame
// Call at the beginning of your render loop, before any ImGui commands
void NewFrame();

// Process wxWidgets mouse event
// Returns true if ImGui wants to capture the mouse
bool ProcessMouseEvent(wxMouseEvent& event);

// Process wxWidgets key event
// Returns true if ImGui wants to capture the keyboard
bool ProcessKeyEvent(wxKeyEvent& event);

// Update display size when window is resized
void UpdateDisplaySize(wxSizeEvent& event);

// Shutdown the wxWidgets backend
void Shutdown();

// Check if ImGui wants to capture mouse events
bool WantCaptureMouse();

// Check if ImGui wants to capture keyboard events
bool WantCaptureKeyboard();

// Apply monocromatic dark theme (VS Code style)
void ApplyDarkTheme();

} // namespace ImGuiWx

#endif // RME_IMGUI_IMPL_WX_H
