# Building Canary Map Editor

This guide provides step-by-step instructions for compiling the Canary Map Editor from source on Linux, Windows, and macOS.

## 1. System Prerequisites

### Required Tools

| Tool      | Minimum Version | Notes                                     |
|-----------|-----------------|-------------------------------------------|
| Git       | 2.x             | For cloning the repository                |
| CMake     | 3.22+           | Build system generator                    |
| C++ Compiler | C++17         | GCC 10+, Clang 12+, or MSVC 2019+       |

### OS-Level System Libraries

These libraries **must be installed on your system** before compiling. CMake will automatically find them.

#### Ubuntu / Debian (apt)

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libwxgtk3.2-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    freeglut3-dev \
    zlib1g-dev \
    liblzma-dev \
    libpugixml-dev \
    protobuf-compiler \
    libprotobuf-dev \
    libxcb1-dev
```

> **Note:** If `libwxgtk3.2-dev` is not available, try `libwxgtk3.0-gtk3-dev`.

#### Fedora / RHEL

```bash
sudo dnf install -y \
    cmake gcc-c++ git \
    wxGTK-devel \
    mesa-libGL-devel mesa-libGLU-devel freeglut-devel \
    zlib-devel xz-devel pugixml-devel \
    protobuf-compiler protobuf-devel \
    libxcb-devel
```

#### macOS (Homebrew)

```bash
brew install cmake git wxwidgets freeglut zlib xz pugixml protobuf
```

#### Windows (vcpkg recommended)

For Windows, using **vcpkg** is the recommended approach:

```powershell
# Clone vcpkg if you don't have it
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg\vcpkg install wxwidgets freeglut zlib liblzma pugixml protobuf --triplet x64-windows
```

Then pass the toolchain to CMake:
```powershell
cmake -B build -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## 2. Automatic Dependencies (No Manual Install Required)

The following libraries are **automatically downloaded and compiled** by CMake via `FetchContent` if they are not found on your system:

| Library         | Version   | Usage                          |
|-----------------|-----------|--------------------------------|
| **Asio**        | 1.30.2    | Networking (header-only)       |
| **fmt**         | 10.1.1    | String formatting              |
| **spdlog**      | 1.12.0    | Logging                        |
| **nlohmann_json** | 3.11.3  | JSON parsing (header-only)     |

This means you **do not need vcpkg** to build on Linux if the OS-level libraries are installed.

---

## 3. Build Instructions

### Standard Build (Release)

```bash
# 1. Clone the repository
git clone https://github.com/your-org/rme_canary.git
cd rme_canary

# 2. Create build directory
mkdir build && cd build

# 3. Configure (Release mode)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. Compile
cmake --build . -j$(nproc)
```

### Debug Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

### Run the Editor

After a successful build, the executable will be in the project root:

```bash
cd ..
./canary-map-editor
```

---

## 4. Troubleshooting

### `Could not find wxWidgets`

**Solution:** Install the wxWidgets development package.
```bash
# Ubuntu/Debian
sudo apt-get install libwxgtk3.2-dev

# Or for older distros
sudo apt-get install libwxgtk3.0-gtk3-dev
```

### `Could not find GLUT`

**Solution:** Install freeglut.
```bash
sudo apt-get install freeglut3-dev
```

### `Could not find Protobuf` or `protobuf::protoc IMPORTED_LOCATION not set`

**Solution:** Install the protobuf compiler and libraries.
```bash
sudo apt-get install protobuf-compiler libprotobuf-dev
```

### `Could not find pugixml`

**Solution:** Install the pugixml development package.
```bash
sudo apt-get install libpugixml-dev
```

### CMake version too old

This project requires CMake 3.22+. On older Ubuntu versions:
```bash
# Install newer CMake via pip
pip install cmake --upgrade

# Or use Kitware's APT repository
# https://apt.kitware.com/
```

---

## 5. Project Structure

| Directory   | Description                              |
|-------------|------------------------------------------|
| `source/`   | Main C++ source code                     |
| `data/`     | Runtime data files (assets, materials)   |
| `build/`    | CMake build output (generated)           |
| `docs/`     | Documentation                            |

---

**Happy Mapping!** 🗺️
