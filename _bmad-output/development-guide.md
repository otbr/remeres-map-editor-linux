# Development Guide

## Build Requirements

**Ubuntu 24.04 LTS (recommended):**

```bash
sudo apt install build-essential cmake git \
                 libwxgtk3.0-gtk3-dev libgl1-mesa-dev \
                 libarchive-dev libglew-dev
```

## Build Instructions

```bash
# Clone repository with submodules (if any, C++ projects often check dependencies or rely on vcpkg)
git clone https://github.com/[your-fork]/canary-map-editor.git
cd canary-map-editor

# Build (CMake)
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)

# Run
./canary-map-editor
```

## IDE Setup (VS Code)

Recommended extensions:
-   **C/C++** (Microsoft)
-   **CMake Tools** (Microsoft)

**Configuration:**
1.  Open the folder in VS Code.
2.  CMake Tools should auto-detect `CMakeLists.txt`.
3.  Select your kit (e.g., GCC 11+ or Clang).
4.  Configure and Build via the status bar.

## Contribrution Guidelines

This is a Linux-focused fork. Contributions welcome for:
-   ✅ Linux/GTK3 bug fixes and optimizations
-   ✅ Cross-distro testing (Arch, Fedora, etc.)
-   ✅ Performance improvements
-   ✅ Documentation updates

Please create pull requests with detailed technical descriptions.
