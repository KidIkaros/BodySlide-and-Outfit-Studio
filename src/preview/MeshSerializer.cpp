/*
BodySlide and Outfit Studio

Three.js Preview Integration - Mesh Serializer

Converts UniversalModel/UniversalMesh meshes to JSON format
for consumption by the Three.js web viewer.
*/

#include <sstream>
#include <iomanip>
#include <fstream>

#include <nlohmann/json.hpp>

#include <components/UniversalModel.h>
#include <preview/MeshSerializer.h>

using json = nlohmann::json;

namespace preview {

std::string MeshJsonData::ToJson() const {
	json j;
	j["vertices"] = this->vertices;
	j["normals"] = this->normals;
	j["uvs"] = this->uvs;
	j["indices"] = this->indices;
	j["colors"] = this->colors;
	j["bonePositions"] = this->bonePositions;
	j["boneNames"] = this->boneNames;
	j["skinWeights"] = this->skinWeights;
	return j.dump(2);
}

MeshJsonData MeshSerializer::SerializeMesh(const univmodel::UniversalMesh& mesh) {
	MeshJsonData data;
	data.name = mesh.name;
	data.vertexCount = static_cast<int>(mesh.vertices.size());
	data.triangleCount = static_cast<int>(mesh.triangles.size());

	data.vertices.reserve(mesh.vertices.size() * 3);
	data.normals.reserve(mesh.vertices.size() * 3);
	data.uvs.reserve(mesh.vertices.size() * 2);
	data.colors.reserve(mesh.vertices.size() * 4);

	for (const auto& v : mesh.vertices) {
		data.vertices.push_back(v.x);
		data.vertices.push_back(v.y);
		data.vertices.push_back(v.z);

		data.normals.push_back(v.nx);
		data.normals.push_back(v.ny);
		data.normals.push_back(v.nz);

		data.uvs.push_back(v.u);
		data.uvs.push_back(v.v);

		data.colors.push_back(v.r);
		data.colors.push_back(v.g);
		data.colors.push_back(v.b);
		data.colors.push_back(v.a);
	}

	data.indices.reserve(mesh.triangles.size() * 3);
	for (const auto& tri : mesh.triangles) {
		data.indices.push_back(tri.v1);
		data.indices.push_back(tri.v2);
		data.indices.push_back(tri.v3);
	}

	if (mesh.skeleton.has_value()) {
		const auto& skeleton = mesh.skeleton.value();
		data.boneNames.reserve(skeleton.bones.size());
		data.bonePositions.reserve(skeleton.bones.size());

		for (const auto& bone : skeleton.bones) {
			data.boneNames.push_back(bone.name);
			std::vector<float> pos = {
				bone.transform[12],
				bone.transform[13],
				bone.transform[14]
			};
			data.bonePositions.push_back(pos);
		}
	}

	if (mesh.skinData.has_value()) {
		const auto& skinData = mesh.skinData.value();
		data.skinWeights.reserve(skinData.size());
		data.skinIndices.reserve(skinData.size());

		for (const auto& vsd : skinData) {
			std::vector<float> weights;
			std::vector<uint32_t> indices;
			weights.reserve(vsd.weights.size() * 2);
			indices.reserve(vsd.weights.size());

			for (const auto& bw : vsd.weights) {
				weights.push_back(static_cast<float>(bw.boneIndex));
				weights.push_back(bw.weight);
				indices.push_back(bw.boneIndex);
			}

			data.skinWeights.push_back(weights);
			data.skinIndices.push_back(indices);
		}
	}

	return data;
}

std::string MeshSerializer::SerializeModel(const univmodel::UniversalModel& model, bool prettyPrint) {
	json j;
	j["meshes"] = json::array();

	for (const auto& mesh : model.meshes) {
		auto meshData = SerializeMesh(mesh);

		json m;
		m["vertexCount"] = meshData.vertexCount;
		m["triangleCount"] = meshData.triangleCount;
		m["vertices"] = meshData.vertices;
		m["normals"] = meshData.normals;
		m["uvs"] = meshData.uvs;
		m["indices"] = meshData.indices;
		m["colors"] = meshData.colors;

		if (!meshData.bonePositions.empty()) {
			m["bones"] = json::array();
			for (size_t i = 0; i < meshData.bonePositions.size(); ++i) {
				json bone;
				bone["name"] = meshData.boneNames[i];
				bone["position"] = meshData.bonePositions[i];
				m["bones"].push_back(bone);
			}
		}

		if (!meshData.skinWeights.empty()) {
			m["skinWeights"] = meshData.skinWeights;
			m["skinIndices"] = meshData.skinIndices;
		}

		j["meshes"].push_back(m);
	}

	j["metadata"] = json::object();
	for (const auto& [k, value] : model.metadata) {
		j["metadata"][k] = value;
	}

	return j.dump(prettyPrint ? 2 : -1);
}

std::string MeshSerializer::SerializeForLivePreview(const univmodel::UniversalMesh& mesh) {
	auto meshData = SerializeMesh(mesh);

	json j;
	j["mesh"] = json::object();
	j["mesh"]["vertices"] = meshData.vertices;
	j["mesh"]["normals"] = meshData.normals;
	j["mesh"]["uvs"] = meshData.uvs;
	j["mesh"]["indices"] = meshData.indices;
	j["mesh"]["bones"] = meshData.bonePositions;
	j["mesh"]["skinWeights"] = meshData.skinWeights;
	j["mesh"]["skinIndices"] = meshData.skinIndices;

	return j.dump();
}

bool MeshSerializer::ExportToGLTF(const univmodel::UniversalModel& model, const std::string& filePath) {
	auto jsonStr = SerializeModel(model, true);

	std::ofstream file(filePath);
	if (!file.is_open()) {
		return false;
	}

	file << jsonStr;
	file.close();

	return true;
}

} // namespace preview