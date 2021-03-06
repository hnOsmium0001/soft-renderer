#include <array>
#include <tuple>
#include <unordered_map>
#include "Model.hpp"

using namespace SRender;

template<>
struct std::hash<Vertex> {
	usize operator()(const Vertex& vert) const {
		usize seed = 0;
		seed ^= MatrixHash<Eigen::Vector3f>()(vert.pos) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= MatrixHash<Eigen::Vector3f>()(vert.normal) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= MatrixHash<Eigen::Vector2f>()(vert.uv) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}
};

auto Mesh::ReadOBJ(std::istream& data) -> Mesh {
	using Vec3fVec = std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>>;
	using Vec2fVec = std::vector<Eigen::Vector2f, Eigen::aligned_allocator<Eigen::Vector2f>>;

	auto mesh = Mesh{};

	char ctrash;
	Vec3fVec posBuf;
	Vec3fVec normalBuf;
	Vec2fVec uvBuf;
	std::unordered_map<Vertex, u32> knownVerts;
	u32 nextID = 0;

	std::string line;
	while (std::getline(data, line)) {
		std::istringstream iss{line};

		std::string start;
		iss >> start;
		if (start == "#") {
			continue;
		} else if (start == "v") {
			Eigen::Vector3f pos;
			for (usize i = 0; i < 3; ++i) iss >> pos[i];
			posBuf.push_back(std::move(pos));
		} else if (start == "vt") {
			Eigen::Vector3f normal;
			for (usize i = 0; i < 3; ++i) iss >> normal[i];
			normalBuf.push_back(std::move(normal));
		} else if (start == "vn") {
			Eigen::Vector2f uv;
			for (usize i = 0; i < 2; ++i) iss >> uv[i];
			uvBuf.push_back(std::move(uv));
		} else if (start == "f") {
			auto tri = std::array<u32, 3>{};
			// We only support triangular faces (with 3 vertices)
			// Format: f pos/uv/normal pos/uv/normal pos/uv/normal
			for (u32 iv, it, in, i = 0; i < 3; ++i) {
				/* A vertex must have at least the position */ iss >> iv;
				iss >> ctrash;
				if (!uvBuf.empty()) iss >> it;
				iss >> ctrash;
				if (!normalBuf.empty()) iss >> in;

				// .obj file index starts at 1
				--iv;
				--it;
				--in;
				auto candidate = Vertex{
					posBuf[iv],
					normalBuf.empty() ? Eigen::Vector3f{} : normalBuf[in],
					uvBuf.empty() ? Eigen::Vector2f{} : uvBuf[it]
				};

				auto iter = knownVerts.find(candidate);
				if (iter != knownVerts.end()) {
					// The vertex already exists, reuse the known version
					tri[i] = iter->second;
				} else {
					// The vertex does not exist, create it
					knownVerts.insert({std::move(candidate), nextID});
					tri[i] = nextID;
					++nextID;
				}
			}
			mesh.indices.insert(mesh.indices.end(), tri.begin(), tri.end());
		} else if (start == "g") {
		}
	}

	mesh.vertices.reserve(knownVerts.size());
	for (const auto& [vert, idx]: knownVerts) {
		mesh.vertices.push_back(vert);
	}
}

auto Mesh::ReadOBJ(std::string_view fileContent) -> Mesh {
	std::istringstream iss(fileContent.data());
	return ReadOBJ(iss);
}

auto Mesh::ReadOBJAt(std::string_view path) -> Mesh {
	std::ifstream ifs;
	ifs.open(path);
	if (!ifs) return;
	return ReadOBJ(ifs);
}
