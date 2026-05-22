PolyForge
=========

A powerful mesh processing and 3D modeling toolkit for creating, converting, and customizing 3D models and outfits.

**Created by:** KidIkaros

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

**Libraries used:**
- OpenGL
- OpenGL Image (GLI)
- Simple OpenGL Image Library 2 (SOIL2)
- TinyXML-2
- FSEngine (BSA/BA2 library)
- Autodesk FBX SDK
- nifly (NIF library)
- wxWidgets
- nlohmann/json

**Build Status:**
- Ubuntu GCC Release: Passing
- Windows x64 Release: Passing

https://github.com/KidIkaros/PolyForge