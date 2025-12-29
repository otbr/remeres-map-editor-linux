# Changelog

## [4.1.0] - 2025-12-29

### Fixed
- **Mountain Rendering:** Resolved "white artifacts" on mountain assets (specifically Item 15298/Sprite 251190) by updating corrupted asset files.
- **Auto Border Editor:** Fixed large sprites (walls) being cropped in previews. They are now correctly scaled to fit the UI.
- **Rendering Quality:** Changed sprite scaling algorithm to `Nearest Neighbor` to preserve pixel art sharpness in the editor UI.

### Changed
- Bumped version to 4.1.0.
