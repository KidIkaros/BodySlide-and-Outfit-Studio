PolyForge
=========

[![CI](https://github.com/KidIkaros/PolyForge/workflows/CMake%20Release/badge.svg)](https://github.com/KidIkaros/PolyForge/actions)
[![License](https://img.shields.io/github/license/KidIkaros/PolyForge)](LICENSE)

A powerful mesh processing and 3D modeling toolkit for creating, converting, and customizing 3D models and outfits.

**Core Features:**
- Edge-aware depth map upsampling for high-quality mesh reconstruction
- Automated mesh optimization with configurable hole-filling
- Camera pose parsing with scientific notation support
- Universal model format handling with validation
- Skeleton creation and bone structure verification

**Technical Highlights:**
- EdgeAwareUpsample: Preserves sharp depth discontinuities during upsampling
- FillHoles: Configurable hole-filling with adaptive size limits
- ParseCameraPoseJSON: Robust JSON parsing for camera pose data
- Comprehensive test coverage for core functionality

**Build Status:**
| Platform | Status |
|----------|--------|
| Ubuntu GCC Release | Passing |
| Windows x64 Release | Passing |

**Libraries used:**
- OpenGL, OpenGL Image (GLI), Simple OpenGL Image Library 2 (SOIL2)
- TinyXML-2, FSEngine (BSA/BA2 library), Autodesk FBX SDK
- nifly (NIF library), wxWidgets, nlohmann/json, GLM

https://github.com/KidIkaros/PolyForge
