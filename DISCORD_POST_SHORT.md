# 🐧 Canary Map Editor v4.0.0 - Linux Release!

**First official public release of the native Linux port of Remere's Map Editor!**

---

## 🚀 What's New?

✨ **Native GTK3 integration** - No Wine, no compatibility layers
⚡ **+567% FPS improvement** - Smooth 60 Hz rendering
🎯 **-98% input lag** - Responsive zoom and pan
🖼️ **Professional desktop integration** - Automatic icon installation
🔍 **Container inspector** - Press `B` to see container contents with tooltips

---

## 📥 Download

**Pre-compiled binary:**
🔗 https://github.com/Habdel-Edenfield/Remeres-map-editor-linux/releases/tag/v4.0.0

```bash
wget https://github.com/Habdel-Edenfield/Remeres-map-editor-linux/releases/download/v4.0.0/canary-map-editor
chmod +x canary-map-editor
./canary-map-editor
```

---

## 🛠️ Quick Install (from source)

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake git libwxgtk3.0-gtk3-dev \
    libgl1-mesa-dev libarchive-dev imagemagick

# Clone, build, install
git clone https://github.com/Habdel-Edenfield/Remeres-map-editor-linux.git
cd Remeres-map-editor-linux
mkdir -p build && cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
cmake --build . -j$(nproc)
cmake --install .
```

**Done!** App appears in your system menu with icons. ✅

---

## 📊 Performance

| Metric | Before | After | Gain |
|--------|--------|-------|------|
| FPS | 9 Hz | 60 Hz | **+567%** |
| Input Lag | 8s | <100ms | **-98%** |
| CPU Usage | 100% | 34% | **-66%** |

---

## 🎯 Best Practices Implemented

✅ **CMake modern build system** with automated icon generation
✅ **freedesktop.org compliance** - proper .desktop file and icon theme
✅ **Multi-size icons** (16px to 256px) for all screen resolutions
✅ **HiDPI support** for retina displays
✅ **Automatic cache updates** (gtk-update-icon-cache, update-desktop-database)
✅ **User & system installation** options (no root required for user install)
✅ **Comprehensive documentation** (README, INSTALL guide, technical docs)

---

## 🔗 Links

- **Repository:** https://github.com/Habdel-Edenfield/Remeres-map-editor-linux
- **Releases:** https://github.com/Habdel-Edenfield/Remeres-map-editor-linux/releases
- **Issues:** https://github.com/Habdel-Edenfield/Remeres-map-editor-linux/issues

---

## 💬 Credits

**Original:** Remere's Map Editor (opentibiabr)
**Linux Port:** Habdel Edenfield
**License:** GPL v3

---

**Happy mapping on Linux!** 🗺️🐧

*#Linux #OpenTibia #MapEditor #GTK3 #OpenSource*
