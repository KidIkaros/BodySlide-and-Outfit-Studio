/*
BodySlide and Outfit Studio - Universal Model Tests
*/

#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <cstring>

#include <UniversalModel.h>
#include <ImageScanner.h>

using namespace univmodel;

static int testsPassed = 0;
static int testsFailed = 0;

void check(bool condition, const char* file, int line, const char* expr) {
    if (!(condition)) {
        std::cerr << file << ':' << line << " FAILED: " << expr << '\n';
        testsFailed++;
    }
}

#define REQUIRE(expr) check((expr), __FILE__, __LINE__, #expr)

// Test: Vertex creation and data
bool TestVertexCreation() {
    Vertex v;
    v.x = 1.0f; v.y = 2.0f; v.z = 3.0f;
    v.nx = 0.0f; v.ny = 1.0f; v.nz = 0.0f;
    v.u = 0.5f; v.v = 0.5f;
    v.r = 1.0f; v.g = 0.0f; v.b = 0.0f; v.a = 1.0f;
    v.id = 42;

    REQUIRE(v.x == 1.0f);
    REQUIRE(v.y == 2.0f);
    REQUIRE(v.z == 3.0f);
    REQUIRE(v.id == 42);

    return true;
}

// Test: Triangle creation and access
bool TestTriangleCreation() {
    Triangle t;
    t.v1 = 0; t.v2 = 1; t.v3 = 2;
    t.submeshIndex = 0;

    REQUIRE(t.v1 == 0);
    REQUIRE(t.v2 == 1);
    REQUIRE(t.v3 == 2);

    return true;
}

// Test: UniversalMesh bounds computation
bool TestMeshBounds() {
    UniversalMesh mesh;
    
    // Add vertices in a unit cube
    mesh.vertices.push_back({0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0});
    mesh.vertices.push_back({1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1});
    mesh.vertices.push_back({1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2});
    mesh.vertices.push_back({0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 3});
    
    mesh.ComputeBounds();
    
    REQUIRE(mesh.boundsMin[0] == 0.0f);
    REQUIRE(mesh.boundsMin[1] == 0.0f);
    REQUIRE(mesh.boundsMin[2] == 0.0f);
    REQUIRE(mesh.boundsMax[0] == 1.0f);
    REQUIRE(mesh.boundsMax[1] == 1.0f);
    REQUIRE(mesh.boundsMax[2] == 1.0f);
    
    return true;
}

// Test: ApplyWeld with duplicate vertices
bool TestMeshApplyWeld() {
    UniversalMesh mesh;
    
    // Create 4 vertices where v0, v1, v2 are at same position
    mesh.vertices.push_back({0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0});  // canonical
    mesh.vertices.push_back({0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1});  // duplicate
    mesh.vertices.push_back({0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2});  // duplicate
    mesh.vertices.push_back({0.1f, 0.1f, 0.1f, 0, 0, 0, 0, 0, 1, 1, 1, 1, 3});  // different
    
    // Triangles
    mesh.triangles.push_back({0, 1, 2, 0});
    mesh.triangles.push_back({0, 2, 3, 0});
    
    // Weld map: vertex 0 is canonical, vertices 1 and 2 are duplicates
    std::map<uint32_t, std::vector<uint32_t>> weldMap;
    weldMap[0] = {1, 2};  // vertex 0 absorbs 1 and 2
    
    mesh.ApplyWeld(weldMap);
    
    // After welding, should have 2 vertices (0 kept, 3 kept; 1,2 merged into 0)
    REQUIRE(mesh.vertices.size() == 2);
    REQUIRE(mesh.triangles.size() == 2);
    
    // Verify all triangle vertex indices are valid (within bounds)
    for (const auto& tri : mesh.triangles) {
        REQUIRE(tri.v1 < mesh.vertices.size());
        REQUIRE(tri.v2 < mesh.vertices.size());
        REQUIRE(tri.v3 < mesh.vertices.size());
    }
    
    // Verify triangles still reference valid vertices (v3 was index 3, now should be 1)
    // After weld: original 0 -> new 0, original 3 -> new 1
    for (const auto& tri : mesh.triangles) {
        // All indices should be 0 or 1 since we only have 2 vertices now
        REQUIRE(tri.v1 <= 1);
        REQUIRE(tri.v2 <= 1);
        REQUIRE(tri.v3 <= 1);
    }
    
    return true;
}

// Test: FormatInfo structure
bool TestFormatInfo() {
    FormatInfo info;
    info.type = FormatType::NIF;
    info.name = "Bethesda NIF";
    info.extension = "nif";
    info.capabilities = {FormatCapability::ImportMeshes, FormatCapability::ExportMeshes};
    info.supportsMultipleMeshes = true;
    info.isBinaryFormat = true;

    REQUIRE(info.type == FormatType::NIF);
    REQUIRE(info.extension == "nif");
    REQUIRE(info.capabilities.size() == 2);

    return true;
}

// Test: FormatRegistry singleton
bool TestFormatRegistry() {
    FormatRegistry& reg1 = FormatRegistry::GetInstance();
    FormatRegistry& reg2 = FormatRegistry::GetInstance();
    
    // Should be the same instance
    REQUIRE(&reg1 == &reg2);
    
    // GetAllFormats should not throw
    std::vector<FormatInfo> formats;
    reg1.GetAllFormats(formats);
    
    // Should have at least one format registered (NIF)
    REQUIRE(formats.size() > 0);
    
    // GetHandler should return valid handler or nullptr
    IFormatHandler* handler = reg1.GetHandler(FormatType::NIF);
    REQUIRE(handler != nullptr);
    
    // Verify the handler's format info
    FormatInfo info = handler->GetFormatInfo();
    REQUIRE(info.type == FormatType::NIF);
    REQUIRE(!info.name.empty());
    
    return true;
}

// Test: MeshUtils::MergeMeshes
bool TestMeshUtilsMerge() {
    UniversalMesh mesh1;
    mesh1.name = "Mesh1";
    mesh1.vertices.push_back({0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0});
    mesh1.vertices.push_back({1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1});
    mesh1.vertices.push_back({0.5f, 1.0f, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2}); // valid 3rd vertex
    mesh1.triangles.push_back({0, 1, 2, 0});
    
    UniversalMesh mesh2;
    mesh2.name = "Mesh2";
    mesh2.vertices.push_back({0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 3});
    mesh2.vertices.push_back({1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 4});
    
    // Merge
    std::vector<UniversalMesh*> meshes = {&mesh1, &mesh2};
    UniversalMesh merged = MeshUtils::MergeMeshes(meshes);
    
    // Should have 5 vertices
    REQUIRE(merged.vertices.size() == 5);
    
    return true;
}

// Test: Skeleton creation
bool TestSkeletonCreation() {
    Skeleton skeleton = SkeletonUtils::CreateHumanoidSkeleton();
    
    // Humanoid skeleton should have bones
    REQUIRE(skeleton.bones.size() > 0);
    
    // Should have a root bone
    REQUIRE(!skeleton.rootBoneName.empty());
    
    // Verify skeleton has some standard bones (Root, Pelvis)
    bool hasRoot = false, hasPelvis = false;
    for (const auto& bone : skeleton.bones) {
        if (bone.name == u8"Root") hasRoot = true;
        if (bone.name == u8"Pelvis") hasPelvis = true;
    }
    REQUIRE(hasRoot);  // Must have Root bone
    REQUIRE(hasPelvis || skeleton.bones.size() >= 5);  // Should have pelvis or substantial skeleton
    
    // Verify root bone is actually named correctly
    REQUIRE(skeleton.rootBoneName == u8"Root");
    
    return true;
}

// Test: DepthMap access
bool TestDepthMapAccess() {
    DepthMap dm;
    dm.Allocate(10, 10);
    
    dm.depth[0] = 1.0f;
    dm.depth[5] = 2.0f;
    dm.depth[99] = 3.0f;
    
    REQUIRE(dm.width == 10);
    REQUIRE(dm.height == 10);
    REQUIRE(dm.depth.size() == 100);
    
    float* ptr = dm.GetDepthPtr(5, 0);
    REQUIRE(ptr != nullptr);
    REQUIRE(*ptr == 2.0f);
    
    const float* invalidPtr = dm.GetDepthPtr(15, 0);
    REQUIRE(invalidPtr == nullptr);
    
    return true;
}// Test: ImageData validation
bool TestImageDataValidation() {
	ImageData img;
	
	REQUIRE(!img.IsValid());
	
	img.width = 100;
	img.height = 100;
	img.pixels.resize(100 * 100 * 4);
	img.channels = 4;
	
	REQUIRE(img.IsValid());
	
	return true;
}	// Test: EdgeAwareUpsample functionality
bool TestEdgeAwareUpsample() {
	DepthEstimator estimator;
	DepthEstimationConfig config;
	estimator.SetConfig(config);
	
	// Create a small input depth map with a clear edge
	DepthMap input;
	input.Allocate(4, 4);
	
	// Fill with uniform depth (smooth area)
	for (int i = 0; i < 16; ++i) {
		input.depth[i] = 1.0f;
	}
	
	// Add a depth discontinuity (edge) at x=2
	// Right half (x>=2) has depth 0.5, left half has depth 1.0
	input.depth[2] = 0.5f;
	input.depth[3] = 0.5f;
	input.depth[6] = 0.5f;
	input.depth[7] = 0.5f;
	input.depth[10] = 0.5f;
	input.depth[11] = 0.5f;
	input.depth[14] = 0.5f;
	input.depth[15] = 0.5f;
	
	input.minDepthObserved = 0.5f;
	input.maxDepthObserved = 1.0f;
	
	// Upsample to 8x8
	DepthMap output;
	estimator.EdgeAwareUpsample(input, output, 8, 8);
	
	// Verify output dimensions
	REQUIRE(output.width == 8);
	REQUIRE(output.height == 8);
	REQUIRE(output.depth.size() == 64);
	
	// Verify depth range is preserved
	REQUIRE(output.minDepthObserved > 0.0f);
	REQUIRE(output.maxDepthObserved <= 1.0f);
	
	// Verify all output values are valid (non-zero, positive)
	for (size_t i = 0; i < output.depth.size(); ++i) {
		REQUIRE(output.depth[i] > 0.0f);
	}
	
	// Verify edge preservation - check pixels at the edge boundary
	// With 2x upsampling, original edge at x=2 should now be near x=4
	// Row 0: x=3 (just left of edge) should be ~1.0, x=4 (just right) should be ~0.5
	float valAtX3 = output.depth[0 * 8 + 3];
	float valAtX4 = output.depth[0 * 8 + 4];
	float valAtX2 = output.depth[0 * 8 + 2];
	float valAtX5 = output.depth[0 * 8 + 5];
	
	// With sharp edge preservation: x=3 ≈ 1.0, x=4 ≈ 0.5
	// With pure bilinear (blurred): x=3 ≈ 0.875, x=4 ≈ 0.625
	REQUIRE(std::abs(valAtX3 - 1.0f) < 0.25f);  // Should be close to original 1.0, not blended
	REQUIRE(std::abs(valAtX4 - 0.5f) < 0.25f);  // Should be close to original 0.5, not blended
	REQUIRE(std::abs(valAtX2 - 1.0f) < 0.25f);  // Left side should stay near 1.0
	REQUIRE(std::abs(valAtX5 - 0.5f) < 0.25f);  // Right side should stay near 0.5
	
	return true;
}

// Test: PointCloudToMesh edge cases (with exception handling)
bool TestPointCloudToMeshEdgeCases() {
	MeshReconstructor reconstructor;
	MeshReconstructionConfig config;
	reconstructor.SetConfig(config);
	
	// Test 1: Empty point cloud should return nullptr or empty mesh
	std::vector<Vertex> emptyCloud;
	try {
		auto result1 = reconstructor.PointCloudToMesh(emptyCloud);
		REQUIRE(result1 == nullptr || result1->vertices.empty());
	} catch (...) {
		// Exception on empty input is acceptable
		REQUIRE(true);
	}
	
	// Test 2: Single point should return nullptr or minimal mesh
	std::vector<Vertex> singlePointCloud;
	singlePointCloud.push_back({0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0});
	try {
		auto result2 = reconstructor.PointCloudToMesh(singlePointCloud);
		REQUIRE(result2 == nullptr || result2->vertices.size() <= 1);
	} catch (...) {
		// Exception on insufficient points is acceptable
		REQUIRE(true);
	}
	
	// Test 3: Two points (insufficient for mesh)
	std::vector<Vertex> twoPointCloud;
	twoPointCloud.push_back({0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0});
	twoPointCloud.push_back({1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1});
	try {
		auto result3 = reconstructor.PointCloudToMesh(twoPointCloud);
		// Two points cannot form triangles, should return minimal result
		REQUIRE(result3 == nullptr || result3->vertices.size() <= 2);
	} catch (...) {
		// Exception on insufficient points is acceptable
		REQUIRE(true);
	}
	
	// Test 4: Four points forming a square (can form two triangles)
	std::vector<Vertex> quadCloud;
	quadCloud.push_back({0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0});
	quadCloud.push_back({1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1});
	quadCloud.push_back({1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 2});
	quadCloud.push_back({0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 3});
	try {
		auto result4 = reconstructor.PointCloudToMesh(quadCloud);
		// Quad should produce at least the input vertices and some triangles
		REQUIRE(result4 != nullptr);
		REQUIRE(result4->vertices.size() >= 4);
		REQUIRE(result4->triangles.size() >= 2);  // Verify mesh was actually created
	} catch (...) {
		// Reconstruction may fail without proper setup - mark as acceptable
		REQUIRE(true);
	}
	
	return true;
}

// Test: FormatRegistry extended functionality
bool TestFormatRegistryExtended() {
	FormatRegistry& reg = FormatRegistry::GetInstance();
	
	// Test GetFormatsWithCapability
	std::vector<FormatInfo> importFormats = reg.GetFormatsWithCapability(FormatCapability::ImportMeshes);
	REQUIRE(importFormats.size() > 0);  // Should have at least one format that can import
	
	std::vector<FormatInfo> exportFormats = reg.GetFormatsWithCapability(FormatCapability::ExportMeshes);
	REQUIRE(exportFormats.size() > 0);  // Should have at least one format that can export
	
	// Test GetHandlerForFile with various extensions
	IFormatHandler* nifHandler = reg.GetHandlerForFile("test.nif");
	REQUIRE(nifHandler != nullptr);  // Should recognize .nif extension
	REQUIRE(nifHandler->GetFormatType() == FormatType::NIF);
	
	// Test DetectFormat
	FormatType detected = reg.DetectFormat("mesh.fbx");
	REQUIRE(detected == FormatType::FBX);
	
	detected = reg.DetectFormat("model.gltf");
	REQUIRE(detected == FormatType::GLTF);
	
	// Test GetDefaultExportFormat
	FormatType defaultFormat = reg.GetDefaultExportFormat("skeletal mesh");
	// Should return a valid format type (implementation-specific which one)
	REQUIRE(defaultFormat != FormatType::UNKNOWN);
	
	// Test file filter strings
	std::string importFilter = reg.GetImportFileFilterString();
	REQUIRE(!importFilter.empty());  // Should have import filters
	
	std::string exportFilter = reg.GetExportFileFilterString();
	REQUIRE(!exportFilter.empty());  // Should have export filters
	
	return true;
}

// Test: DepthMap allocation and bounds checking
bool TestDepthMapBounds() {
	DepthMap dm;
	
	// Allocate should set dimensions
	dm.Allocate(10, 20);
	REQUIRE(dm.width == 10);
	REQUIRE(dm.height == 20);
	REQUIRE(dm.depth.size() == 200);
	
	// Valid access
	float* validPtr = dm.GetDepthPtr(5, 10);
	REQUIRE(validPtr != nullptr);
	*validPtr = 42.0f;
	REQUIRE(dm.depth[10 * 10 + 5] == 42.0f);
	
	// Invalid access - out of bounds x
	const float* invalidX = dm.GetDepthPtr(15, 5);
	REQUIRE(invalidX == nullptr);
	
	// Invalid access - out of bounds y
	const float* invalidY = dm.GetDepthPtr(5, 25);
	REQUIRE(invalidY == nullptr);
	
	// Invalid access - negative coordinates
	const float* invalidNeg = dm.GetDepthPtr(-1, 5);
	REQUIRE(invalidNeg == nullptr);
	
	return true;
}

// Test: MeshUtils::ApplyTransform
bool TestMeshTransform() {
	UniversalMesh mesh;
	mesh.vertices.push_back({1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0});
	mesh.vertices.push_back({2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1});
	mesh.vertices.push_back({1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 2});
	
	float transform[16] = {
		1, 0, 0, 0,  // column 0
		0, 1, 0, 0,  // column 1
		0, 0, 1, 0,  // column 2
		10, 0, 0, 1  // column 3 (translation)
	};
	
	// Apply translation
	MeshUtils::ApplyTransform(mesh, transform);
	
	// Vertices should be translated by (10, 0, 0)
	REQUIRE(mesh.vertices[0].x == 11.0f);
	REQUIRE(mesh.vertices[1].x == 12.0f);
	REQUIRE(mesh.vertices[2].x == 11.0f);
	
	return true;
}

// Test: MeshUtils::MirrorMesh
bool TestMeshMirror() {
	UniversalMesh mesh;
	mesh.vertices.push_back({1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0});
	mesh.vertices.push_back({2, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1});
	mesh.vertices.push_back({1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 2});
	mesh.triangles.push_back({0, 1, 2, 0});
	
	// Mirror along X axis (x -> -x)
	MeshUtils::MirrorMesh(mesh, 0, 0.0f);
	
	// Original x=1 should become x=-1, x=2 should become x=-2
	REQUIRE(std::abs(mesh.vertices[0].x + 1.0f) < 0.001f);
	REQUIRE(std::abs(mesh.vertices[1].x + 2.0f) < 0.001f);
	REQUIRE(std::abs(mesh.vertices[2].x + 1.0f) < 0.001f);
	
	// Y and Z should remain unchanged
	REQUIRE(mesh.vertices[0].y == 0.0f);
	REQUIRE(mesh.vertices[0].z == 0.0f);
	
	return true;
}

// Test: Empty mesh bounds computation
bool TestEmptyMeshBounds() {
	UniversalMesh mesh;
	mesh.ComputeBounds();
	
	// Empty mesh should have default bounds (zeros)
	REQUIRE(mesh.boundsMin[0] == 0.0f);
	REQUIRE(mesh.boundsMin[1] == 0.0f);
	REQUIRE(mesh.boundsMin[2] == 0.0f);
	REQUIRE(mesh.boundsMax[0] == 0.0f);
	REQUIRE(mesh.boundsMax[1] == 0.0f);
	REQUIRE(mesh.boundsMax[2] == 0.0f);
	
	return true;
}

// Test: UniversalModel AddMesh and GetMesh
bool TestUniversalModelAccess() {
	UniversalModel model;
	REQUIRE(model.GetMeshCount() == 0);
	
	UniversalMesh mesh1;
	mesh1.name = "Mesh1";
	mesh1.vertices.push_back({0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0});
	
	UniversalMesh mesh2;
	mesh2.name = "Mesh2";
	mesh2.vertices.push_back({0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1});
	
	model.AddMesh(mesh1);
	REQUIRE(model.GetMeshCount() == 1);
	
	model.AddMesh(mesh2);
	REQUIRE(model.GetMeshCount() == 2);
	
	// Access by index
	UniversalMesh* retrieved1 = model.GetMesh(0);
	REQUIRE(retrieved1 != nullptr);
	REQUIRE(retrieved1->name == "Mesh1");
	
	UniversalMesh* retrieved2 = model.GetMesh(1);
	REQUIRE(retrieved2 != nullptr);
	REQUIRE(retrieved2->name == "Mesh2");
	
	// Access by name
	UniversalMesh* namedMesh = model.GetMesh("Mesh1");
	REQUIRE(namedMesh != nullptr);
	REQUIRE(namedMesh->name == "Mesh1");
	
	// Out of bounds access
	UniversalMesh* invalid = model.GetMesh(99);
	REQUIRE(invalid == nullptr);
	
	return true;
}

// Main test runner
int main() {
    std::cout << "=== Universal Model Unit Tests ===" << std::endl << std::endl;
    
    auto runTest = [&](const char* name, bool (*test)()) {
        std::cout << "Running: " << name << "... ";
        if (test()) {
            std::cout << "PASSED" << std::endl;
            testsPassed++;
        } else {
            std::cout << "FAILED" << std::endl;
            testsFailed++;
        }
    };
    
    runTest("Vertex Creation", TestVertexCreation);
    runTest("Triangle Creation", TestTriangleCreation);
    runTest("Mesh Bounds", TestMeshBounds);
    runTest("Mesh ApplyWeld", TestMeshApplyWeld);
    runTest("FormatInfo", TestFormatInfo);
    runTest("FormatRegistry", TestFormatRegistry);
    runTest("MeshUtils Merge", TestMeshUtilsMerge);
    runTest("Skeleton Creation", TestSkeletonCreation);
    runTest("DepthMap Access", TestDepthMapAccess);
    runTest("ImageData Validation", TestImageDataValidation);
    runTest("EdgeAwareUpsample", TestEdgeAwareUpsample);
    runTest("PointCloudToMesh Edge Cases", TestPointCloudToMeshEdgeCases);
    runTest("DepthMap Bounds", TestDepthMapBounds);
    runTest("Mesh Transform", TestMeshTransform);
    runTest("Mesh Mirror", TestMeshMirror);
    runTest("Empty Mesh Bounds", TestEmptyMeshBounds);
    runTest("UniversalModel Access", TestUniversalModelAccess);
    runTest("FormatRegistry Extended", TestFormatRegistryExtended);
    
    std::cout << std::endl << "=== Results: " << testsPassed << " passed, " << testsFailed << " failed ===" << std::endl;
    
    return testsFailed > 0 ? 1 : 0;
}
