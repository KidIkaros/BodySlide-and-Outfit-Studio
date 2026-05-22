# Changelog

All notable changes to PolyForge will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Added
- **EdgeAwareUpsample**: High-quality depth map upsampling with edge preservation
  - Edge detection to identify depth discontinuities
  - Nearest neighbor interpolation near edges to preserve sharp transitions
  - Bilinear interpolation in smooth areas for efficiency
  - Confidence map upsampling included
  - Configurable epsilon threshold for robustness

- **ParseCameraPoseJSON**: Robust camera pose parsing with full JSON support
  - Scientific notation support (1e-5)
  - Graceful fallback to identity matrix on parse failure
  - Uses nlohmann::json for proper parsing

- **TestEdgeAwareUpsample**: Unit test for edge-aware depth map upsampling

- **Comprehensive Documentation**:
  - README.md with project overview and features
  - docs/PROJECT.md with detailed usage examples and API documentation
  - CONTRIBUTING.md with contribution guidelines

### Improved
- **OptimizeMesh efficiency**: Adjacency map is now built once before the smoothing loop instead of rebuilding every iteration. Reduces time complexity from O(iterations × triangles) to O(triangles).

- **FillHoles**: Uses configurable `maxHoleSize` from `MeshReconstructionConfig` instead of hardcoded 1000 limit. Allows customization based on mesh requirements.

- **Test improvements**:
  - `ApplyWeld` test now verifies triangle vertex indices are valid after remapping
  - `SkeletonCreation` test verifies bone structure (Root bone, Pelvis or 5+ bones)
  - `FormatRegistry` test validates handler exists and format info is correct

- **CMakeLists.txt**: Now checks for nifly as CMake target first before falling back to `find_library`. Ensures proper integration with the nifly submodule.

### Fixed
- **ParseCameraPoseJSON comment**: Updated misleading comment in catch block from \"Fall back to simple parsing for non-JSON files\" to accurate description \"JSON parsing failed - LoadCameraPoses will use identity matrix as fallback\"

## [0.1.0] - 2025-01-01

### Added
- Initial release as PolyForge (forked from BodySlide-and-Outfit-Studio)
- Core mesh processing components
- Universal model format support
- Format registry system