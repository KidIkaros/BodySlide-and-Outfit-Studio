# PolyForge - 3D Mesh Processing Toolkit

PolyForge is a powerful open-source toolkit for mesh processing and 3D model manipulation. Originally derived from BodySlide-and-Outfit-Studio, PolyForge has evolved into a general-purpose mesh processing library and application.

## Overview

PolyForge provides tools and libraries for:
- **Mesh Reconstruction**: Edge-aware depth map upsampling with preservation of sharp depth discontinuities
- **Mesh Optimization**: Configurable hole-filling with adaptive size limits and efficient smoothing
- **Camera Pose Handling**: Robust JSON parsing supporting scientific notation (1e-5)
- **Universal Model Support**: Format detection and validation for 3D model files

## Key Features

### EdgeAwareUpsample
High-quality depth map upsampling that preserves sharp edges and depth discontinuities. Uses a hybrid approach:
- Edge detection to identify depth discontinuities
- Nearest neighbor interpolation near edges to preserve sharp transitions
- Bilinear interpolation in smooth areas for efficiency

### FillHoles
Automatic mesh hole-filling with configurable parameters:
- Adaptive hole size limits (configurable via MeshReconstructionConfig)
- Respects geometry boundaries
- Configurable max hole size for safety

### ParseCameraPoseJSON
Robust camera pose parsing:
- Full JSON support with scientific notation (1e-5)
- Graceful fallback when parsing fails
- Used for multi-view 3D reconstruction

### OptimizeMesh
Efficient mesh smoothing with:
- Pre-computed adjacency maps (built once, not per iteration)
- Configurable iteration count
- Automatic degenerate triangle removal

## Building

### Prerequisites
- C++17 compatible compiler
- CMake 3.10+
- wxWidgets 3.2+
- OpenGL 4.1+
- Git with submodules

### Ubuntu/Debian
```bash
sudo apt-get install build-essential cmake libwxgtk3.2-dev libglew-dev libgl1-mesa-dev
git clone https://github.com/KidIkaros/PolyForge.git
cd PolyForge
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows (Visual Studio 2022+)
```powershell
git clone https://github.com/KidIkaros/PolyForge.git
cd PolyForge
mkdir build && cd build
cmake .. -G “Visual Studio 17 2022” -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build . --config Release
```

## Usage Examples

### C++ API Usage

```cpp
#include “components/ImageScanner.h”

// Edge-aware depth map upsampling
DepthEstimator estimator;
DepthMap input, output;
input.Allocate(256, 256);
// ... populate input.depth with depth values ...
estimator.EdgeAwareUpsample(input, output, 512, 512);

// Optimize mesh with hole filling
MeshReconstructor reconstructor;
MeshReconstructionConfig config;
config.maxHoleSize = 500;  // Custom hole size limit
reconstructor.SetConfig(config);
reconstructor.FillHoles(mesh);
```

### Camera Pose Parsing

```cpp
#include “components/ImageScanner.h”

ImageScanner scanner;
auto poses = scanner.LoadCameraPoses(“camera_poses.json”);
// Returns vector of 4x4 transformation matrices
// Falls back to identity matrix if parsing fails
```

### Universal Model Format Detection

```cpp
#include “components/UniversalModel.h”

UniversalModel model;
FormatRegistry registry;
auto format = registry.DetectFormat(“mesh.nif”);
if (format) {
    model.Load(“mesh.nif”, format);
}
```

## Architecture

PolyForge is built around several core components:

- **ImageScanner**: Depth estimation and image processing
- **MeshReconstructor**: 3D mesh generation from depth maps
- **UniversalModel**: Cross-format 3D model support
- **FormatRegistry**: Plugin-based format handler system

## Testing

Run the test suite:
```bash
cd build
ctest --output-on-failure
```

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please see CONTRIBUTING.md for guidelines.