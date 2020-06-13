#pragma once

#include <vector>
#include <Eigen/Dense>
#include <tl/expected.hpp>
#include "Util.hpp"
#include "TGAImage.hpp"

namespace SRender {

class FrameBuffer {
public:
	TGAImage image;
	std::vector<f32> depthBuffer;

public:
	static auto Merge(std::vector<FrameBuffer> layers) -> tl::expected<FrameBuffer, std::string>;

	FrameBuffer(u32 width, u32 height);
	FrameBuffer(const FrameBuffer&) = default;
	FrameBuffer& operator=(const FrameBuffer&) = default;
	FrameBuffer(FrameBuffer&&) = default;
	FrameBuffer& operator=(FrameBuffer&&) = default;

	auto Get(u32 x, u32 y) const -> TGAColor;
	auto Set(u32 x, u32 y, TGAColor color) -> bool;
	auto GetDepth(u32 x, u32 y) const -> f32;
	auto SetDepth(u32 x, u32 y, f32 z) -> void;
};

class MaskFrameBuffer : public FrameBuffer {
public:
	bool depthTest;
	bool depthMask;

public:
	MaskFrameBuffer(u32 width, u32 height);
	MaskFrameBuffer(const MaskFrameBuffer&) = default;
	MaskFrameBuffer& operator=(const MaskFrameBuffer&) = default;
	MaskFrameBuffer(MaskFrameBuffer&&) = default;
	MaskFrameBuffer& operator=(MaskFrameBuffer&&) = default;

	auto Get(u32 x, u32 y) const -> TGAColor;
	auto Set(u32 x, u32 y, f32 z, TGAColor color) -> void;
};

template <class V>
Eigen::Matrix<V, 3, 1> Barycentric(
	const Eigen::Matrix<V, 3, 1>& pt,
	const Eigen::Matrix<V, 3, 1>& v1,
	const Eigen::Matrix<V, 3, 1>& v2,
	const Eigen::Matrix<V, 3, 1>& v3
) {
	using Vec = Eigen::Matrix<V, 3, 1>;
	Vec u = Vec(v3.x() - v1.x(), v2.x() - v1.x(), v1.x() - pt.x())
		.cross(Vec(v3.y() - v1.y(), v2.y() - v1.y(), v1.y() - pt.y()));

	return std::abs(u.z()) < 1
		? Vec(-1, -1, -1)
		: Vec(1.0f - (u.x() + u.y()) / u.z(), u.y() / u.z(), u.x() / u.z());
}

} // namespace SRender
