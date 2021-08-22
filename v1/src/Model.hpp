#pragma once

#include <string>
#include <vector>
#include <istream>
#include <Eigen/Dense>
#include "Util.hpp"

namespace SRender {
class Vertex {
public:
	Eigen::Vector3f pos;
	Eigen::Vector3f normal;
	Eigen::Vector2f uv;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	auto operator==(const Vertex&) const -> bool = default;
};
} // namespace SRender

template<>
struct std::hash<SRender::Vertex>;


namespace SRender {
class Mesh {
private:
	using VerticesVec = std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>>;
	using IndicesVec = std::vector<usize>;

	VerticesVec vertices;
	IndicesVec indices;

public:
	static auto ReadOBJ(std::istream& data) -> Mesh;
	static auto ReadOBJ(std::string_view fileContent) -> Mesh;
	static auto ReadOBJAt(std::string_view path) -> Mesh;

	auto Vertices() -> VerticesVec& { return vertices; }
	auto Vertices() const -> const VerticesVec& { return vertices; }
	auto Indices() -> IndicesVec& { return indices; }
	auto Indices() const -> const IndicesVec& { return indices; }
};
} // namespace SRender

