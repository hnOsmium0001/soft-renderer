#pragma once

#include <functional>
#include <vector>
#include <span>
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
	
	auto RenderTriangle(
		const Eigen::Vector3f& p1,
		const Eigen::Vector3f& p2,
		const Eigen::Vector3f& p3,
		const std::function<auto(const Eigen::Vector2f&) -> TGAColor>& frag
	) -> void;

	auto RenderTriangles(
		std::span<Eigen::Vector3f> vertices,
		std::span<usize> indices,
		const std::function<auto(const Eigen::Vector2f&) -> TGAColor>& frag
	) -> void;

	auto RenderLine(
		const Eigen::Vector3f& a,
		const Eigen::Vector3f& b,
		TGAColor color
	) -> void;

	auto GetWidth() const -> u32 { return image.GetWidth(); }
	auto GetHeight() const -> u32 { return image.GetHeight(); }
};

class RenderBuffer : public FrameBuffer {
public:
	bool depthTest;
	bool depthMask;

public:
	RenderBuffer(u32 width, u32 height);
	RenderBuffer(const RenderBuffer&) = default;
	RenderBuffer& operator=(const RenderBuffer&) = default;
	RenderBuffer(RenderBuffer&&) = default;
	RenderBuffer& operator=(RenderBuffer&&) = default;

	auto Set(u32 x, u32 y, f32 z, TGAColor color) -> void;
};

auto Barycentric(
	const Eigen::Vector3f& pt,
	const Eigen::Vector3f& v1,
	const Eigen::Vector3f& v2,
	const Eigen::Vector3f& v3
) -> Eigen::Vector3f;

auto Barycentric(
	const Eigen::Vector2f& pt,
	const Eigen::Vector2f& v1,
	const Eigen::Vector2f& v2,
	const Eigen::Vector2f& v3
) -> Eigen::Vector3f;

} // namespace SRender
