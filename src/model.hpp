#pragma once

#include <vector>
#include <Eigen/Dense>

class Model {
private:
	std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>> verts_;
	std::vector<std::vector<int>> faces_;
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Eigen::Vector3f vert(int i);
	const Eigen::Vector3f& vert(int i) const;
	std::vector<int> face(int idx);
	const std::vector<int>& face(int idx) const;
};
