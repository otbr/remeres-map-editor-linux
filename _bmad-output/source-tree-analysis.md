# Source Tree Analysis

## Project Structure

This project follows a standard C++ CMake monolith structure, with source code separated from assets and build configuration.

```
rme_canary/
├── source/                  # Core Application Source Code
│   ├── protobuf/            # Protobuf definitions and generated code
│   ├── gui.h/cpp            # Main GUI controller (Singleton)
│   ├── editor.cpp           # Editor state management
│   ├── map.cpp              # Map data structure and logic
│   ├── map_drawer.cpp       # OpenGL rendering pipeline
│   ├── item.cpp             # Item entity logic
│   ├── iomap_otbm.cpp       # OTBM Map serialization (I/O)
│   ├── main_menubar.cpp     # wxWidgets Menu implementation
│   └── imgui_panels.cpp     # ImGui panel implementation
├── data/                    # Application Assets & Configuration
│   ├── assets/              # JSON catalog content
│   ├── materials/           # XML brush definitions (grounds, walls, etc.)
│   ├── clients.xml          # Client version definitions
│   └── menubar.xml          # Menu structure definition
├── cmake/                   # CMake modules and scripts
├── docs/                    # Project Documentation
├── build/                   # CMake build directory (git-ignored)
├── CMakeLists.txt           # Main build configuration
├── vcpkg.json               # Dependency manifest
└── README.md                # Project entry point
```

## Critical Directories

### Source Analysis (`source/`)

The `source/` directory contains 240+ files implementing the editor's logic. Key components include:

-   **Core Logic**: `editor.cpp`, `application.cpp`, `main.cpp`
-   **Data Model**: `map.cpp`, `tile.cpp`, `item.cpp`, `town.cpp`
-   **Rendering**: `map_drawer.cpp`, `light_drawer.cpp`, `map_display.cpp` (wxGLCanvas)
-   **IO**: `iomap_otbm.cpp` (OTBM), `iomap_otmm.cpp`, `filehandle.cpp`
-   **GUI (wxWidgets)**: `main_menubar.cpp`, `main_toolbar.cpp`, `common_windows.cpp`
-   **GUI (ImGui)**: `imgui_panels.cpp`, `imgui_impl_wx.cpp`
-   **Brushes**: `brush.cpp` and specific brush implementations (`ground_brush.cpp`, `wall_brush.cpp`)

### Asset Management (`data/`)

The `data/` directory is critical for the editor's functionality, converting raw data into usable editor brushes and palettes:

-   **`materials/`**: Defines the "brushes" available to the user (Grounds, Walls, Doodads).
-   **`assets/`**: Contains `catalog-content.json` for asset metadata.
-   **`clients.xml`**: critical for supporting multiple Tibia client versions.

## Integration Points

-   **vcpkg Integration**: Dependencies are managed via `vcpkg.json` and linked in `CMakeLists.txt`.
-   **OpenGL/Graphics**: Integrated via `wxGLCanvas` in `MapDisplay` and `ImGui` rendering into that canvas.
