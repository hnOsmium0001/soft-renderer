#pragma once

#include <string>
#include <vector>
#include <istream>
#include <Eigen/Dense>
#include "Util.hpp"

namespace SRender {

class Vertex {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Eigen::Vector3f pos;
	Eigen::Vector3f normal;
	Eigen::Vector2f uv;
};

class Mesh {
private:
	std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>> vertices;
	std::vector<usize> indices;

public:
	static auto ReadOBJ(Mesh& target, std::istream& data) -> void;
	static auto ReadOBJ(Mesh& target, std::string_view data) -> void;
	static auto ReadOBJAt(Mesh& target, std::string_view path) -> void;
};

} // namespace SRender
