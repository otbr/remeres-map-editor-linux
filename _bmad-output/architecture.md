# Architecture Documentation

## Executive Summary

Canary Map Editor (Linux Port) is a specialized fork of Remere's Map Editor, optimized for the Linux desktop environment using GTK3. It employs a hybrid architecture combining **wxWidgets** for native OS integration (menus, dialogs, window management) and **ImGui** for high-performance, immediate-mode tool panels. Rendering is handled via **OpenGL** on a custom `wxGLCanvas`.

## Architecture Pattern

**Type:** Event-driven Desktop Application

The application moves away from a continuous game-loop style rendering (common in games) to an **event-driven model**. Rendering only occurs when:
1.  The user interacts with the canvas (mouse/keyboard events).
2.  Data changes (brush selection, map updates).
3.  OS window events occur (resize, focus).

This reduces CPU/GPU usage significantly when idle.

## Technology Stack

| Category | Technology | Description |
| :--- | :--- | :--- |
| **Language** | C++ 20 | Core application logic. |
| **Build System** | CMake | Cross-platform build configuration. |
| **Dependency Manager** | vcpkg | Manages library dependencies (fmt, spdlog, etc.). |
| **GUI Framework** | wxWidgets 3.0+ | Native windowing, menus, file dialogs. |
| **UI Panels** | Dear ImGui | Dockable, hardware-accelerated tool windows. |
| **Graphics API** | OpenGL 3.3+ | Map rendering (via `MapDrawer`). |
| **Serialization** | OTBM / Protobuf | Map data formats and asset definitions. |

## Core Subsystems

### 1. The Editor Core (`Editor`, `Map`)
-   **`Editor`**: The central controller class. Manages the state of the application, active map tabs, and tools.
-   **`Map`**: Represents a single open map file. Contains the tile data (`Tile`, `Item`) and metadata (Towns, Spawns).
-   **`Action` System**: Implements the Command Pattern for Undo/Redo functionality.

### 2. Rendering Pipeline (`MapDrawer`)
-   Located in `source/map_drawer.cpp`.
-   Iterates over visible tiles on the `MapCanvas`.
-   Draws ground, items, and creatures using OpenGL calls.
-   **Optimization**: Uses Z-axis occlusion culling to skip rendering objects covered by opaque floors above them.

### 3. User Interface (Hybrid)
-   **Main Window (`MainFrame`)**: A `wxFrame` that hosts the `wxAuiManager`.
-   **Docking System**: Uses `wxAUI` to manage dockable panes.
-   **ImGui Loop**: A dedicated rendering pass (`imgui_impl_wx`) injects ImGui draw commands into the OpenGL context, allowing for overlay UIs and floating tool windows inside the canvas.

### 4. Asset Management
-   **`GraphicManager`**: Loads and manages sprites (`.spr`) and texture atlases.
-   **`Materials`**: XML-based definitions in `data/materials/` map logical brushes (e.g., "Stone Wall") to specific Item IDs.

## Data Flow

1.  **Input**: User clicks map -> `MapCanvas` receives wxEvent.
2.  **Logic**: `GroundBrush` (or valid tool) calculates changes.
3.  **Command**: An `Action` is created (e.g., `ActionDrawTile`) and pushed to the history.
4.  **Update**: Map data is modified in memory.
5.  **Render**: `g_gui.RefreshView()` is called, triggering a repaint of the damaged region.
