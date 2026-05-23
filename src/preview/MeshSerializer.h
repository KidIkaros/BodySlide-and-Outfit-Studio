/*
BodySlide and Outfit Studio

Three.js Preview Integration - Mesh Serializer

Converts UniversalModel/UnivModel meshes to JSON format
for consumption by the Three.js web viewer.
*/

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace univmodel {
	class UniversalMesh;
	class UniversalModel;
}

namespace preview {

struct MeshJsonData {
	// Vertices: [x1, y1, z1, x2, y2, z2, ...]
	std::vector<float> vertices;
	
	// Normals: [nx1, ny1, nz1, nx2, ny2, nz2, ...]
	std::vector<float> normals;
	
	// UVs: [u1, v1, u2, v2, ...]
	std::vector<float> uvs;
	
	// Indices: [i1, i2, i3, i4, ...]
	std::vector<uint32_t> indices;
	
	// Bone positions for skeleton visualization: [[x,y,z], [x,y,z], ...]
	std::vector<std::vector<float>> bonePositions;
	
	// Bone names
	std::vector<std::string> boneNames;
	
	// Skin weights (vertex bone assignments)
	std::vector<std::vector<float>> skinWeights;  // Per-vertex: [boneIdx, weight, boneIdx, weight, ...]
	std::vector<std::vector<uint32_t>> skinIndices;
	
	// Vertex colors: [r1, g1, b1, a1, r2, g2, b2, a2, ...]
	std::vector<float> colors;
	
	// Mesh metadata
	std::string name;
	int vertexCount = 0;
	int triangleCount = 0;
	
	// Convert to JSON string
	std::string ToJson() const;
};

class MeshSerializer {
public:
	// Serialize a single mesh
	static MeshJsonData SerializeMesh(const univmodel::UniversalMesh& mesh);
	
	// Serialize entire model (all meshes)
	static std::string SerializeModel(const univmodel::UniversalModel& model, bool prettyPrint = false);
	
	// Serialize for WebSocket live preview (minimal payload)
	static std::string SerializeForLivePreview(const univmodel::UniversalMesh& mesh);
	
	// Create GLTF-compatible buffer (for export to GLTF file)
	static bool ExportToGLTF(const univmodel::UniversalModel& model, const std::string& filePath);
	
private:
	static void SerializeVertex(const univmodel::Vertex& v, MeshJsonData& data);
	static void AppendWeldMap(MeshJsonData& data, const std::map<uint32_t, std::vector<uint32_t>>& weldMap);
};

} // namespace preview