# Project Overview

## Canary Map Editor (Linux Port)

**Canary Map Editor** is a native Linux fork of the renowned Remere's Map Editor (RME), designed for creating maps for OpenTibia (OT) servers. This project focuses on stability, performance, and seamless integration with the Linux desktop environment through a modern GTK3 port.

### Key Characteristics

-   **Type**: Monolithic Desktop Application
-   **Language**: C++ 20
-   **Architecture**: Event-driven, Hybrid GUI (wxWidgets + ImGui)
-   **Platform**: Linux (Primary), Windows (Legacy)

### Technology Stack Summary

| Component | Technology | Role |
| :--- | :--- | :--- |
| **GUI** | wxWidgets 3.0+ | Native windows, dialogs, menus |
| **Tools** | Dear ImGui | Dockable panels, immediate mode widget rendering |
| **Graphics** | OpenGL 3.3+ | Hardware-accelerated map rendering |
| **Build** | CMake | Build system configuration |
| **Package** | vcpkg | Dependency management |

### Repository Structure

This is a **Monolithic** repository containing the full application source and assets.

-   **`source/`**: C++ application logic.
-   **`data/`**: Runtime assets (materials, catalog).
-   **`docs/`**: Project documentation (this folder).

### Documentation Navigation

-   **[Architecture Guide](./architecture.md)**: Deep dive into the event-driven rendering model and class structure.
-   **[Source Tree Analysis](./source-tree-analysis.md)**: Detailed breakdown of folders and critical files.
-   **[Development Guide](./development-guide.md)**: How to build, run, and contribute to the project.
