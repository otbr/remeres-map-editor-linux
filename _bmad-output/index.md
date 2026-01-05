# Documentation Index

## Project Documentation Index

### Project Overview

- **Type:** Monolithic Desktop Application
- **Primary Language:** C++ 20
- **Architecture:** Event-driven Desktop Application (ImGui/wxWidgets Hybrid)

### Quick Reference

- **Tech Stack:** C++ 20, CMake 3.22+, vcpkg, wxWidgets (GUI), ImGui (Rendering), OpenGL, Protobuf
- **Entry Point:** `source/main.cpp` (via `application.cpp`)
- **Architecture Pattern:** Event-driven Desktop Application (ImGui/wxWidgets Hybrid)

### Generated Documentation

- [Project Overview](./project-overview.md)
- [Architecture](./architecture.md)
- [Source Tree Analysis](./source-tree-analysis.md)
- [Development Guide](./development-guide.md)

### Existing Documentation

- [README.md](../README.md) - Main project entry point and features list.

### Getting Started

To build the project on Linux (Ubuntu 24.04 recommended):

```bash
sudo apt install build-essential cmake git libwxgtk3.0-gtk3-dev libgl1-mesa-dev libarchive-dev libglew-dev
git clone [repo]
mkdir build && cd build
cmake .. && cmake --build . -j$(nproc)
```

See [Development Guide](./development-guide.md) for details.
